#include "ModelPanel.h"

#include "Tool_Defines.h"

#include "EngineUtility.h"
#include "MapScene.h"
#include "IEHelper.h"
#include "Object.h"
#include "Model.h"

ModelPanel::ModelPanel(const string& PanelName, bool open)
	:Panel{PanelName, open}
{
}

HRESULT ModelPanel::Initialize()
{
    if (FAILED(m_pModelObject = m_pEngineUtility->FindObject(SCENE::MODEL, TEXT("Test"), 0)))
        return E_FAIL;
    SafeAddRef(m_pModelObject);

    m_PanelPosition = _float2(50.f, 50.f);
    m_PanelSize = _float2(400.f, 800.f);

    return S_OK;
}

void ModelPanel::OnRender()
{
    if (ImGui::Button("MapScene"))
    {
        m_pEngineUtility->ChangeScene(SCENE::MAP, MapScene::Create(SCENE::MAP));
        m_pEngineUtility->SetPanelOpen(GetPanelName(), false);
    }
    ImGui::Text("\n");
    static string ImportPath = "None";
    if (ImGui::Button("ImportFBX"))
    {
        OPENFILENAME FileName = {};
        _tchar filePath[MAX_PATH] = {};

        FileName.lStructSize = sizeof(FileName);
        FileName.hwndOwner = GetActiveWindow();
        FileName.lpstrFile = filePath;
        FileName.nMaxFile = sizeof(filePath);
        FileName.lpstrFilter = L"All Files\0*.*\0";
        FileName.nFilterIndex = 1;
        FileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        
        if (GetOpenFileName(&FileName))
        {
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
            string fbxPath(size_needed - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, &fbxPath[0], size_needed, nullptr, nullptr);

            if (filesystem::exists(fbxPath))
            {
                ModelData* model = nullptr;
                if (IEHelper::ImportFBX(fbxPath, *model))
                {
                    Model* pModel = dynamic_cast<Model*>(m_pModelObject->FindComponent(TEXT("Model")));
                    if(pModel != nullptr)
                        pModel->SetModelData(model);
                    ImportPath = fbxPath;
                }
                else
                {
                    MessageBoxA(nullptr, "Failed to import FBX", "Import Error", MB_OK | MB_ICONERROR);
                }
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("ImportBin"))
    {
        OPENFILENAME FileName = {};
        _tchar filePath[MAX_PATH] = {};

        FileName.lStructSize = sizeof(FileName);
        FileName.hwndOwner = GetActiveWindow();
        FileName.lpstrFile = filePath;
        FileName.nMaxFile = sizeof(filePath);
        FileName.lpstrFilter = L"All Files\0*.*\0";
        FileName.nFilterIndex = 1;
        FileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileName(&FileName))
        {
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
            string binPath(size_needed - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, filePath, -1, &binPath[0], size_needed, nullptr, nullptr);

            if (filesystem::exists(binPath))
            {
                ModelData* model = new ModelData();

                Model* pModel = dynamic_cast<Model*>(m_pModelObject->FindComponent(TEXT("Model")));
                if (pModel != nullptr)
                {
                    ModelData* loadedModel = pModel->LoadNoAssimpModel(binPath.c_str());
                    if (loadedModel != nullptr)
                    {
                        pModel->SetModelData(loadedModel);
                        ImportPath = binPath;
                    }
                    else
                    {
                        MessageBoxA(nullptr, "Failed to import Bin", "Import Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }
    }
    ImGui::Text("--Ixported ModelFilePath--");
    ImGui::Text("%s", ImportPath.c_str());
    ImGui::Text("\n");
    static string ExportPath = "None";
    if (ImGui::Button("ExportBin"))
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
                ExportPath = savePath;
            }
        }
    }
    ImGui::Text("--Exported BinaryFilePath--");
    ImGui::Text("%s", ExportPath.c_str());
    ImGui::Text("\n");

    ModelData* currentModel = nullptr;
    Model* pModel = dynamic_cast<Model*>(m_pModelObject->FindComponent(TEXT("Model")));
    if (pModel != nullptr)
        currentModel = pModel->GetModelData();
    if (currentModel == nullptr) return;
    ImGui::Text("--CurrentModel Data--");
    ImGui::Text("iNumBones : %d", currentModel->bones.size());
    ImGui::Text("iNumMeshes : %d", currentModel->meshes.size());
    ImGui::Text("iNumMaterials : %d", currentModel->materials.size());
    ImGui::Text("iNumAnimations : %d", currentModel->animations.size());

    static _int iTargetMeshIndex = -1;
    ImGui::Text("--TargetMeshIndex(Default : -1)--");
    if(ImGui::InputInt("##TargetMeshIndex", &iTargetMeshIndex))
    {
        pModel->SetTargetMesh(iTargetMeshIndex);
    }
}


ModelPanel* ModelPanel::Create(const string& PanelName, bool open)
{
    ModelPanel* pInstance = new ModelPanel(PanelName, open);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : ModelPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void ModelPanel::Free()
{
    __super::Free();
    SafeRelease(m_pModelObject);
}