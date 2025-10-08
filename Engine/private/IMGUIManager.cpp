#include "IMGUIManager.h"

#include "EngineUtility.h"
#include <IMGUI/imgui.h>
#include <IMGUI/backends/imgui_impl_win32.h>
#include <IMGUI/backends/imgui_impl_dx11.h>

IMGUIManager::IMGUIManager()
    : m_pEngineUtility( EngineUtility::GetInstance() )
{
    SafeAddRef(m_pEngineUtility);
}

IMGUIManager* IMGUIManager::Create()
{
    IMGUIManager* pInstance = new IMGUIManager;
    return pInstance;
}

HRESULT IMGUIManager::Initialize(HWND hWnd)
{

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

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

HRESULT IMGUIManager::Show()
{
    // 예시 UI
    static bool showDemo = true;
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);

    ImGui::Begin("Engine IMGUI");
    ImGui::Text("Hello from Engine DLL!");
    ImGui::End();

    return S_OK;
}

HRESULT IMGUIManager::Render()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return S_OK;
}

HRESULT IMGUIManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return S_OK;
}

IMGUIManager* IMGUIManager::Create()
{
    return new IMGUIManager();
}

void IMGUIManager::Free()
{
    __super::Free();

    m_pEngineUtility->DestroyInstance();
}
