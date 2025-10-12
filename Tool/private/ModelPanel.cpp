#include "ModelPanel.h"

#include "Tool_Defines.h"

#include "EngineUtility.h"
#include "MapScene.h"
#include "ImportHelper.h"

ModelPanel::ModelPanel(const string& PanelName, bool open)
	:Panel{PanelName, open}
{

}

void ModelPanel::OnRender()
{
    ImGui::Begin(GetPanelName().c_str(), &m_IsOpen);

    if (ImGui::Button("Import"))
    {
        // Windows 파일 선택 다이얼로그
        OPENFILENAME ofn = {};
        char szFile[MAX_PATH] = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "FBX Files\0*.fbx\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileName(&ofn))
        {
            std::string fbxPath = szFile;
            if (std::filesystem::exists(fbxPath))
            {
                ModelData model;
                if (ImportHelper::ImportFBX(fbxPath, model))
                {
                    m_pEngineUtility->SetCurrentModel(model);
                }
                else
                {
                    MessageBoxA(nullptr, "Failed to import FBX", "Import Error", MB_OK | MB_ICONERROR);
                }
            }
        }
    }

    if (ImGui::Button("Export"))
    {
        ModelData currentModel;
        if (m_pEngineUtility->GetCurrentModel(currentModel))
        {
            // 간단한 Save As 다이얼로그
            OPENFILENAME ofn = {};
            char szFile[MAX_PATH] = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = GetActiveWindow();
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "Binary Model Files\0*.bin\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn))
            {
                std::ofstream ofs(szFile, std::ios::binary);
                if (ofs.is_open())
                {
                    uint32_t meshCount = static_cast<uint32_t>(currentModel.meshes.size());
                    ofs.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
                    for (const auto& mesh : currentModel.meshes)
                    {
                        uint32_t nameLen = static_cast<uint32_t>(mesh.name.size());
                        ofs.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
                        ofs.write(mesh.name.c_str(), nameLen);

                        uint32_t vCount = static_cast<uint32_t>(mesh.positions.size());
                        ofs.write(reinterpret_cast<const char*>(&vCount), sizeof(vCount));
                        ofs.write(reinterpret_cast<const char*>(mesh.positions.data()), vCount * sizeof(_float3));
                    }
                }
            }
        }
    }

    if (ImGui::Button("Move to MapScene"))
    {
        m_pEngineUtility->ChangeScene(SCENE::MAP, MapScene::Create(SCENE::MAP));
        m_pEngineUtility->SetPanelOpen(GetPanelName(), false);
    }

    ImGui::End();
}


ModelPanel* ModelPanel::Create(const string& PanelName, bool open)
{
    return new ModelPanel(PanelName, open);
}

void ModelPanel::Free()
{
    __super::Free();
}