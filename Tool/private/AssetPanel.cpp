#include "AssetPanel.h"

#include "Tool_Defines.h"

AssetPanel::AssetPanel(const string& PanelName, bool open)
    :Panel{ PanelName, open }
{
}
HRESULT AssetPanel::Initialize()
{
    m_RootDirectory = std::filesystem::current_path();
    m_CurrentDirectory = m_RootDirectory;
    return S_OK;
}

void AssetPanel::OnRender()
{
    ImGui::Text("Current: %s", m_CurrentDirectory.string().c_str());
    ImGui::SameLine();
    if (ImGui::Button("Up") && m_CurrentDirectory != m_RootDirectory)
        m_CurrentDirectory = m_CurrentDirectory.parent_path();

    ImGui::Separator();

    // 파일 리스트
    for (auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
    {
        const auto& path = entry.path();
        std::string name = path.filename().string();

        if (entry.is_directory())
        {
            if (ImGui::Selectable((name + "/").c_str()))
                m_CurrentDirectory /= name;
        }
        else
        {
            bool isSelected = (m_SelectedFile == name);
            if (ImGui::Selectable(name.c_str(), isSelected))
                m_SelectedFile = name;

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                // 더블클릭 시 미리보기 or 로드
                //if (path.extension() == ".fbx")
                //    m_pEngineUtility->SetModelData(path.string());
                //else if (path.extension() == ".png" || path.extension() == ".dds")
                //    m_pEngineUtility->PreviewTexture(path.string());
            }
        }
    }
}

AssetPanel* AssetPanel::Create(const string& PanelName, bool open)
{
    AssetPanel* pInstance = new AssetPanel(PanelName, open);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : AssetPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void AssetPanel::Free()
{
    __super::Free();
}
