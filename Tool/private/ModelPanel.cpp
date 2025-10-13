#include "ModelPanel.h"

#include "Tool_Defines.h"

#include "EngineUtility.h"
#include "MapScene.h"
#include "IEHelper.h"
#include "Object.h"
#include "Model.h"

ModelPanel::ModelPanel(const string& PanelName, Object* pObject, bool open)
	:Panel{PanelName, open}, m_pModelObject(pObject)
{
    SafeAddRef(m_pModelObject);
}

void ModelPanel::OnRender()
{
    if (ImGui::Button("MapScene"))
    {
        m_pEngineUtility->ChangeScene(SCENE::MAP, MapScene::Create(SCENE::MAP));
        m_pEngineUtility->SetPanelOpen(GetPanelName(), false);
    }

    static string ImportPath = "None";
    if (ImGui::Button("Import"))
    {
        OPENFILENAME FileName = {};
        _tchar filePath[MAX_PATH] = {};

        FileName.lStructSize = sizeof(FileName);
        FileName.hwndOwner = GetActiveWindow();
        FileName.lpstrFile = filePath;
        FileName.nMaxFile = sizeof(filePath);
        FileName.lpstrFilter = L"FBX Files\0*.fbx\0All Files\0*.*\0";
        FileName.nFilterIndex = 1;
        FileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        
        if (GetOpenFileName(&FileName))
        {
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
            string fbxPath(size_needed - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, &fbxPath[0], size_needed, nullptr, nullptr);

            if (filesystem::exists(fbxPath))
            {
                ModelData model;
                if (IEHelper::ImportFBX(fbxPath, model))
                {
                    Model* pModel = dynamic_cast<Model*>(m_pModelObject->FindComponent(TEXT("Model")));
                    if(pModel != nullptr)
                        pModel->SetModelData(&model);
                    ImportPath = fbxPath;
                }
                else
                {
                    MessageBoxA(nullptr, "Failed to import FBX", "Import Error", MB_OK | MB_ICONERROR);
                }
            }
        }
    }
    ImGui::Text("CurrentModelFilePath : %s", ImportPath.c_str());

    static string ExportPath = "None";
    if (ImGui::Button("Export"))
    {
        ModelData* currentModel = nullptr;
        Model* pModel = dynamic_cast<Model*>(m_pModelObject->FindComponent(TEXT("Model")));
        if (pModel != nullptr)
            currentModel = pModel->GetModelData();

        if (currentModel != nullptr)
        {
            OPENFILENAMEW FileName = {};
            wchar_t filePath[MAX_PATH] = {};
            FileName.lStructSize = sizeof(FileName);
            FileName.hwndOwner = GetActiveWindow();
            FileName.lpstrFile = filePath;
            FileName.nMaxFile = MAX_PATH;
            FileName.lpstrFilter = L"Binary Model Files\0*.bin\0All Files\0*.*\0";
            FileName.nFilterIndex = 1;
            FileName.Flags = OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameW(&FileName))
            {
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
                string savePath(size_needed - 1, 0);
                WideCharToMultiByte(CP_UTF8, 0, filePath, -1, &savePath[0], size_needed, nullptr, nullptr);

                IEHelper::ExportModel(savePath, *currentModel);
            }
        }
    }
    ImGui::Text("BinaryFilePath : %s", ExportPath.c_str());
}


ModelPanel* ModelPanel::Create(const string& PanelName, Object* pObject, bool open)
{
    return new ModelPanel(PanelName, pObject, open);
}

void ModelPanel::Free()
{
    __super::Free();
    SafeRelease(m_pModelObject);
}