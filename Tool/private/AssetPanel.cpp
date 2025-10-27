#include "AssetPanel.h"

#include "Tool_Defines.h"
#include "EngineUtility.h"
#include "MapPanel.h"

AssetPanel::AssetPanel(const string& PanelName, bool open)
    :Panel{ PanelName, open }
{
}
HRESULT AssetPanel::Initialize()
{
    m_RootDirectory = std::filesystem::current_path();
    m_CurrentDirectory = m_RootDirectory;

    m_PanelPosition = _float2(0.f, 300.f);
    m_PanelSize = _float2(400.f, 450.f);

    return S_OK;
}

void AssetPanel::OnRender()
{
    ImGui::Text("Current: %s", m_CurrentDirectory.string().c_str());
    
    if (ImGui::Button("UpperPath") && m_CurrentDirectory != m_RootDirectory)
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
                std::string ext = path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                    ext == ".bmp" || ext == ".tga" || ext == ".dds")
                {
                    LoadPreviewTexture(path);
                }
                else if (ext == ".bin")
                {
                    // MapPanel에 경로 전달
                    MapPanel* pMapPanel = dynamic_cast<MapPanel*>(m_pEngineUtility->FindPanel("MapPanel"));
                    if (pMapPanel)
                    {
                        pMapPanel->SetSelectedBinPath(path.string());
                    }
                }
                else if (ext == ".dat")
                {
                    MapPanel* pMapPanel = dynamic_cast<MapPanel*>(m_pEngineUtility->FindPanel("MapPanel"));
                    if (pMapPanel)
                    {
                        pMapPanel->SetNavigationDataPath(path.string());
                    }
                }

            }
        }
    }

    ImGui::SeparatorText("Preview");
    if (m_pPreviewShaderResourceView)
    {
        ImVec2 previewSize(256, 256);
        ImGui::Image((ImTextureID)m_pPreviewShaderResourceView, previewSize);
    }
    else
    {
        ImGui::TextDisabled("No texture selected");
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

    if(m_pPreviewShaderResourceView)
        SafeRelease(m_pPreviewShaderResourceView);
}

_bool AssetPanel::LoadPreviewTexture(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        return false;

    ID3D11Device* device = m_pEngineUtility->GetDevice();
    ID3D11DeviceContext* context = m_pEngineUtility->GetContext();

    if (!device || !context)
        return false;

    // 이전 리소스 해제
    if (m_pPreviewShaderResourceView)
        SafeRelease(m_pPreviewShaderResourceView);

    std::wstring ext = path.extension().wstring();
    for (auto& c : ext) c = towlower(c);

    HRESULT hr = E_FAIL;

    // DDS
    if (ext == L".dds")
    {
        hr = CreateDDSTextureFromFile(device, context, path.c_str(), nullptr, &m_pPreviewShaderResourceView);
    }
    // 그 외 (PNG, JPG, BMP, TGA 등)
    else
    {
        hr = CreateWICTextureFromFile(device, context, path.c_str(), nullptr, &m_pPreviewShaderResourceView);
    }

    if (FAILED(hr))
    {
        OutputDebugStringW((L"[AssetPanel] Failed to load preview texture: " + path.wstring() + L"\n").c_str());
        return false;
    }

    return true;
}
