#include "IMGUIManager.h"

#include "EngineUtility.h"
#include "Panel.h"

IMGUIManager::IMGUIManager()
    : m_pEngineUtility( EngineUtility::GetInstance() )
{
    SafeAddRef(m_pEngineUtility);
}

IMGUIManager* IMGUIManager::Create(HWND hWnd)
{
    IMGUIManager* pInstance = new IMGUIManager();

    if (FAILED(pInstance->Initialize(hWnd)))
    {
        MSG_BOX("Failed to Created : IMGUIManager");
        SafeRelease(pInstance);
    }

    return pInstance;;
}

HRESULT IMGUIManager::Initialize(HWND hWnd)
{

    IMGUI_CHECKVERSION();
    m_IMGUIContext = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hWnd))
        return E_FAIL;
    if (!ImGui_ImplDX11_Init(m_pEngineUtility->GetDevice(), m_pEngineUtility->GetContext()))
        return E_FAIL;

    return S_OK;
}

HRESULT IMGUIManager::Begin()
{   
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    return S_OK;
}

HRESULT IMGUIManager::Render()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    for (auto it = m_Panels.begin(); it != m_Panels.end(); )
    {
        if (it->second == nullptr || it->second->IsOpen() == false)
        {
            SafeRelease(it->second);
            it = m_Panels.erase(it);
        }
        else
            ++it;
    }

    return S_OK;
}

HRESULT IMGUIManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return S_OK;
}

HRESULT IMGUIManager::AddPanel(const string& PanelName, Panel* pPanel)
{
    if (!pPanel)
        return E_FAIL;

    if (m_Panels.find(PanelName) != m_Panels.end())
        return E_FAIL; // 이미 존재하는 이름

    SafeAddRef(pPanel);
    m_Panels.emplace(PanelName, pPanel);
    return S_OK;
}

HRESULT IMGUIManager::SetPanelOpen(const string& PanelName, bool open)
{
    if (m_Panels.find(PanelName) == m_Panels.end())
        return E_FAIL;

    m_Panels.find(PanelName)->second->SetOpen(open);
    return S_OK;
}

HRESULT IMGUIManager::RemovePanel(const string& PanelName)
{
    auto it = m_Panels.find(PanelName);
    if (it == m_Panels.end())
        return E_FAIL;

    SafeRelease(it->second);
    m_Panels.erase(it);
    return S_OK;
}

HRESULT IMGUIManager::ClearPanels()
{
    for (auto& [name, panel] : m_Panels)
    {
        if (panel)
            SafeRelease(panel);
    }
    m_Panels.clear();
    return S_OK;
}

ImGuiContext* IMGUIManager::GetIMGUIContext() const
{
    return m_IMGUIContext;
}

void IMGUIManager::DrawPanels()
{
    for (auto& [name, panel] : m_Panels)
    {
        if (panel && panel->IsOpen())
            panel->Draw();
    }
}

void IMGUIManager::Free()
{
    __super::Free();

    ClearPanels();

    SafeRelease(m_pEngineUtility);
}
