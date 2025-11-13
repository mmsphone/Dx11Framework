#include "LightPanel.h"

#include "EngineUtility.h"

#include "Light.h"
#include "ShadowLight.h"

LightPanel::LightPanel(const std::string& panelName, bool open)
	:Panel{panelName, open}
{
}

HRESULT LightPanel::Initialize()
{
	m_PanelPosition = _float2(900.f, 200.f);
	m_PanelSize = _float2(400.f, 500.f);

	return S_OK;
}

void LightPanel::OnRender()
{
    ImGui::Text("Light Panel");
    ImGui::Separator();

    list<Light*> lights = m_pEngineUtility->GetAllLights();
    const int lightCount = static_cast<int>(lights.size());

    ImGui::Text("Lights (%d)", lightCount);

    ImGui::BeginChild("LightList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

    int idx = 0;
    Light* clickedLight = nullptr;

    for (auto* pLight : lights)
    {
        char label[64];
        sprintf_s(label, "Light_%d", idx);

        bool selected = (idx == m_SelectedLightIndex && pLight == m_pSelectedLight);

        if (ImGui::Selectable(label, selected))
        {
            m_SelectedLightIndex = idx;
            m_pSelectedLight = pLight;
            clickedLight = pLight;
        }
        ++idx;
    }

    ImGui::EndChild();

    // 선택 변경 시, 데이터 동기화
    if (clickedLight)
        SyncFromSelectedLight();

    ImGui::Separator();

    // ----------------------------------------
    // 2) 선택된 라이트 편집
    // ----------------------------------------
    if (m_pSelectedLight && m_SelectedLightIndex >= 0 && m_SelectedLightIndex < lightCount)
    {
        ImGui::Text("Selected Light: %d", m_SelectedLightIndex);

        // 타입 선택
        const char* typeItems[] = { "Directional", "Point", "Spot" };
        ImGui::Combo("Type##Edit", &m_LightType, typeItems, IM_ARRAYSIZE(typeItems));

        // 색상
        ImGui::ColorEdit4("Diffuse##Edit", &m_Diffuse.x);
        ImGui::ColorEdit4("Ambient##Edit", &m_Ambient.x);
        ImGui::ColorEdit4("Specular##Edit", &m_Specular.x);

        // 위치 / 방향 / 범위
        ImGui::DragFloat4("Position##Edit", &m_Position.x, 0.1f);
        if (m_LightType == 0) // Directional
        {
            ImGui::DragFloat4("Direction##Edit", &m_Direction.x, 0.01f, -1.f, 1.f, "%.3f");
        }
        else // Point, Spot
        {
            ImGui::DragFloat("Range##Edit", &m_Range, 0.1f, 0.0f, 10000.0f);
        }

        if (m_LightType == 2) // Spot
        {
            ImGui::DragFloat("Inner (deg)##Edit", &m_InnerDeg, 0.1f, 0.0f, 179.9f);
            ImGui::DragFloat("Outer (deg)##Edit", &m_OuterDeg, 0.1f, 0.0f, 179.9f);
            if (m_InnerDeg > m_OuterDeg)
                std::swap(m_InnerDeg, m_OuterDeg);

            ImGui::DragFloat4("Spot Dir##Edit", &m_Direction.x, 0.01f, -1.f, 1.f, "%.3f");
        }

        // 적용 / 삭제 버튼
        if (ImGui::Button("Apply##Light"))
        {
            ApplyToSelectedLight();
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete##Light"))
        {
            DeleteSelectedLight();
        }
    }
    else
    {
        ImGui::TextDisabled("No light selected.");
    }

    ImGui::Separator();
    ImGui::Text("Shadow Lights");

    std::list<ShadowLight*> shadowLights = m_pEngineUtility->GetAllShadowLights();
    const int shadowCount = static_cast<int>(shadowLights.size());
    ImGui::Text("ShadowLights (%d)", shadowCount);

    ImGui::BeginChild("ShadowLightList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

    int sIdx = 0;
    ShadowLight* clickedShadow = nullptr;

    for (auto* pShadow : shadowLights)
    {
        char label[64];
        sprintf_s(label, "ShadowLight_%d", sIdx);

        bool selected = (sIdx == m_SelectedShadowIndex && pShadow == m_pSelectedShadow);

        if (ImGui::Selectable(label, selected))
        {
            m_SelectedShadowIndex = sIdx;
            m_pSelectedShadow = pShadow;
            clickedShadow = pShadow;
        }
        ++sIdx;
    }

    ImGui::EndChild();

    // 선택 변경 시 동기화
    if (clickedShadow)
        SyncFromSelectedShadowLight();

    ImGui::Separator();

    if (m_pSelectedShadow && m_SelectedShadowIndex >= 0 && m_SelectedShadowIndex < shadowCount)
    {
        ImGui::Text("Selected ShadowLight: %d", m_SelectedShadowIndex);

        ImGui::DragFloat3("Eye##ShadowEdit", &m_EditShadowEye.x, 0.1f);
        ImGui::DragFloat3("At##ShadowEdit", &m_EditShadowAt.x, 0.1f);
        ImGui::DragFloat("FOVy (deg)##ShadowEdit", &m_EditShadowFovy, 0.1f, 1.f, 179.9f);
        ImGui::DragFloat("Near##ShadowEdit", &m_EditShadowNear, 0.001f, 0.001f, 10.f);
        ImGui::DragFloat("Far##ShadowEdit", &m_EditShadowFar, 0.01f, 0.01f, 10000.f);

        if (ImGui::Button("Apply##ShadowLight"))
        {
            ApplyToSelectedShadowLight();
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete##ShadowLight"))
        {
            DeleteSelectedShadowLight();
        }
    }
    else
    {
        ImGui::TextDisabled("No shadow light selected.");
    }


    ImGui::Separator();
    ImGui::Text("Create New Lights");

    // 0=Directional, 1=Point, 2=Spot
    static int   s_lightType = 1;
    static bool  s_castShadow = true;

    // 공통 컬러/강도
    static XMFLOAT4 s_diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.f);
    static XMFLOAT4 s_ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.f);
    static XMFLOAT4 s_specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.f);

    // 수동 입력 포지션
    static XMFLOAT3 s_pos = XMFLOAT3(0.f, 5.f, 0.f);

    // Direction/Spot forward
    static XMFLOAT4 s_dir = XMFLOAT4(0.f, -1.f, 0.f, 0.f);

    // Point/Spot
    static float s_range = 15.f;

    // Spot 각도(도 단위)
    static float s_inner_deg = 20.f;  // degrees
    static float s_outer_deg = 40.f;  // degrees

    // Shadow 파라미터 (각도: 도 단위, aspect는 뷰포트에서 자동)
    static XMFLOAT3 s_eye = XMFLOAT3(0.f, 6.f, 0.f);
    static XMFLOAT3 s_at = XMFLOAT3(0.f, 5.f, -1.f);
    static float    s_fovy_deg = 75.f;     // degrees
    static float    s_znear = 0.1f;
    static float    s_zfar = 50.f;

    // 현재 화면비 구하기(가드 포함)
    auto GetAspectNow = [&]() -> float {
        float aspect = 16.f / 9.f;
        UINT nView = 0;
        m_pEngineUtility->GetContext()->RSGetViewports(&nView, nullptr);
        if (nView > 0) {
            std::vector<D3D11_VIEWPORT> views(nView);
            m_pEngineUtility->GetContext()->RSGetViewports(&nView, views.data());
            if (views[0].Width > 0.f && views[0].Height > 0.f)
                aspect = views[0].Width / views[0].Height;
        }
        return aspect;
        };

    ImGui::Combo("Type##New", &s_lightType, "Directional\0Point\0Spot\0");
    ImGui::Checkbox("Cast Shadow##New", &s_castShadow);

    ImGui::ColorEdit4("Diffuse##New", &s_diffuse.x);
    ImGui::ColorEdit4("Ambient##New", &s_ambient.x);
    ImGui::ColorEdit4("Specular##New", &s_specular.x);
    ImGui::DragFloat3("Position##New", &s_pos.x, 0.1f);

    if (s_lightType == 0) {
        ImGui::DragFloat4("Direction (w ignored)", &s_dir.x, 0.01f, -1.f, 1.f, "%.3f");
    }
    else {
        ImGui::DragFloat("Range##New", &s_range, 0.1f, 0.0f, 10000.0f);
    }

    if (s_lightType == 2) {
        // Spot: 각도 입력을 '도'로 노출
        ImGui::DragFloat("Inner (deg)##New", &s_inner_deg, 0.1f, 0.0f, 179.9f);
        ImGui::DragFloat("Outer (deg)##New", &s_outer_deg, 0.1f, 0.0f, 179.9f);
        if (s_inner_deg > s_outer_deg) std::swap(s_inner_deg, s_outer_deg);
        ImGui::DragFloat4("Forward (w ignored)##New", &s_dir.x, 0.01f, -1.f, 1.f, "%.3f");
    }

    if (s_castShadow) {
        ImGui::SeparatorText("Shadow");
        ImGui::DragFloat3("Eye##New", &s_eye.x, 0.1f);
        ImGui::DragFloat3("At##New", &s_at.x, 0.1f);
        ImGui::DragFloat("FOVy (deg)##New", &s_fovy_deg, 0.1f, 1.f, 179.9f);
        ImGui::DragFloat("Near##New", &s_znear, 0.001f, 0.001f, 10.0f);
        ImGui::DragFloat("Far##New", &s_zfar, 0.01f, 0.01f, 10000.0f);

        // 참고용 화면비 표시(저장 시 자동 적용)
        ImGui::TextDisabled("Aspect (auto): %.4f", GetAspectNow());
    }

    if (ImGui::Button("Add Light (Manual Pos)")) {
        // LIGHT_DESC
        LIGHT_DESC L{};
        L.vDiffuse = s_diffuse;
        L.vAmbient = s_ambient;
        L.vSpecular = s_specular;

        if (s_lightType == 0) {
            L.eType = LIGHT_DIRECTIONAL;
            XMVECTOR d = XMVector3Normalize(XMLoadFloat4(&s_dir));
            XMFLOAT3 dn; XMStoreFloat3(&dn, d);
            L.vDirection = XMFLOAT4(dn.x, dn.y, dn.z, 0.f);
            L.vPosition = XMFLOAT4(s_pos.x, s_pos.y, s_pos.z, 1.f);
        }
        else if (s_lightType == 1) {
            L.eType = LIGHT_POINT;
            L.vDirection = XMFLOAT4(0, 0, 0, 0);
            L.vPosition = XMFLOAT4(s_pos.x, s_pos.y, s_pos.z, 1.f);
            L.fRange = s_range;
        }
        else { // Spot
            L.eType = LIGHT_SPOTLIGHT;
            XMVECTOR d = XMVector3Normalize(XMLoadFloat4(&s_dir));
            XMFLOAT3 dn; XMStoreFloat3(&dn, d);
            L.vDirection = XMFLOAT4(dn.x, dn.y, dn.z, 0.f);
            L.vPosition = XMFLOAT4(s_pos.x, s_pos.y, s_pos.z, 1.f);
            L.fRange = s_range;
            // deg -> rad 변환
            L.fInnerCone = XMConvertToRadians(s_inner_deg);
            L.fOuterCone = XMConvertToRadians(s_outer_deg);
        }

        if (FAILED(m_pEngineUtility->AddLight(L))) {
            MessageBoxA(nullptr, "AddLight failed.", "Light", MB_OK);
        }
        else if (s_castShadow) {
            // SHADOW_DESC (deg->rad 변환, aspect는 현재 화면에서 자동)
            SHADOW_DESC S{};
            S.vEye = s_eye;
            S.vAt = s_at;
            S.fFovy = XMConvertToRadians(s_fovy_deg);
            S.fNear = s_znear;
            S.fFar = s_zfar;
            S.fAspect = GetAspectNow(); // ★ 현재 화면비로 저장

            if (FAILED(m_pEngineUtility->AddShadowLight(S))) {
                MessageBoxA(nullptr, "AddShadowLight failed.", "Light", MB_OK);
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save Lights...")) {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Light Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameA(&ofn)) {
            if (FAILED(m_pEngineUtility->SaveLights(szFile))) {
                MessageBoxA(nullptr, "SaveLights failed.", "Light", MB_OK);
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Lights...")) {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Light Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameA(&ofn)) {
            if (FAILED(m_pEngineUtility->ReadyLightsFromFile(szFile))) {
                MessageBoxA(nullptr, "LoadLights failed.", "Light", MB_OK);
            }
        }
    }
}

LightPanel* LightPanel::Create(const string& PanelName, bool open)
{
    LightPanel* pInstance = new LightPanel(PanelName, open);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : LightPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void LightPanel::Free()
{
    __super::Free();
}

void LightPanel::SyncFromSelectedLight()
{
    if (!m_pSelectedLight)
        return;

    const LIGHT_DESC* pDesc = m_pSelectedLight->GetLight();
    if (!pDesc)
        return;

    m_LightType = static_cast<_int>(pDesc->eType);
    m_Diffuse = pDesc->vDiffuse;
    m_Ambient = pDesc->vAmbient;
    m_Specular = pDesc->vSpecular;
    m_Direction = pDesc->vDirection;
    m_Position = pDesc->vPosition;
    m_Range = pDesc->fRange;

    if (pDesc->eType == LIGHT_SPOTLIGHT)
    {
        m_InnerDeg = XMConvertToDegrees(pDesc->fInnerCone);
        m_OuterDeg = XMConvertToDegrees(pDesc->fOuterCone);
    }
    else
    {
        m_InnerDeg = 20.f;
        m_OuterDeg = 40.f;
    }
}

void LightPanel::ApplyToSelectedLight()
{
    if (!m_pSelectedLight)
        return;

    LIGHT_DESC* pDesc = const_cast<LIGHT_DESC*>(m_pSelectedLight->GetLight());
    if (!pDesc)
        return;

    pDesc->vDiffuse = m_Diffuse;
    pDesc->vAmbient = m_Ambient;
    pDesc->vSpecular = m_Specular;
    pDesc->vPosition = m_Position;
    pDesc->fRange = m_Range;

    if (m_LightType == 0)
    {
        pDesc->eType = LIGHT_DIRECTIONAL;
        XMVECTOR d = XMVector3Normalize(XMLoadFloat4(&m_Direction));
        XMFLOAT3 dn; XMStoreFloat3(&dn, d);
        pDesc->vDirection = XMFLOAT4(dn.x, dn.y, dn.z, 0.f);
        pDesc->fInnerCone = 0.f;
        pDesc->fOuterCone = 0.f;
    }
    else if (m_LightType == 1)
    {
        pDesc->eType = LIGHT_POINT;
        pDesc->vDirection = XMFLOAT4(0, 0, 0, 0);
        pDesc->fInnerCone = 0.f;
        pDesc->fOuterCone = 0.f;
    }
    else
    {
        pDesc->eType = LIGHT_SPOTLIGHT;
        XMVECTOR d = XMVector3Normalize(XMLoadFloat4(&m_Direction));
        XMFLOAT3 dn; XMStoreFloat3(&dn, d);
        pDesc->vDirection = XMFLOAT4(dn.x, dn.y, dn.z, 0.f);

        pDesc->fInnerCone = XMConvertToRadians(m_InnerDeg);
        pDesc->fOuterCone = XMConvertToRadians(m_OuterDeg);
    }
}

void LightPanel::DeleteSelectedLight()
{
    if (!m_pSelectedLight || m_SelectedLightIndex < 0)
        return;

    // EngineUtility 쪽에 RemoveLight(_uint index) 래핑이 있다고 가정
    // 없으면 LightManager::RemoveLight을 래핑해서 하나 만들어주면 됨.
    if (FAILED(m_pEngineUtility->RemoveLight(static_cast<_uint>(m_SelectedLightIndex))))
    {
        MessageBoxA(nullptr, "RemoveLight failed.", "Light", MB_OK);
        return;
    }

    m_SelectedLightIndex = -1;
    m_pSelectedLight = nullptr;
}

void LightPanel::SyncFromSelectedShadowLight()
{
    if (!m_pSelectedShadow)
        return;

    const SHADOW_DESC* pDesc = m_pSelectedShadow->GetShadowLight();
    if (!pDesc)
        return;

    m_EditShadowEye = pDesc->vEye;
    m_EditShadowAt = pDesc->vAt;
    m_EditShadowFovy = XMConvertToDegrees(pDesc->fFovy);
    m_EditShadowNear = pDesc->fNear;
    m_EditShadowFar = pDesc->fFar;
}

void LightPanel::ApplyToSelectedShadowLight()
{
    if (!m_pSelectedShadow)
        return;

    // 기존 값을 베이스로 수정
    SHADOW_DESC newDesc = *m_pSelectedShadow->GetShadowLight();

    newDesc.vEye = m_EditShadowEye;
    newDesc.vAt = m_EditShadowAt;
    newDesc.fFovy = XMConvertToRadians(m_EditShadowFovy);
    newDesc.fNear = m_EditShadowNear;
    newDesc.fFar = m_EditShadowFar;

    m_pSelectedShadow->UpdateFromDesc(newDesc);
}

void LightPanel::DeleteSelectedShadowLight()
{
    if (!m_pSelectedShadow || m_SelectedShadowIndex < 0)
        return;

    if (FAILED(m_pEngineUtility->RemoveShadowLight(static_cast<_uint>(m_SelectedShadowIndex))))
    {
        MessageBoxA(nullptr, "RemoveShadowLight failed.", "ShadowLight", MB_OK);
        return;
    }

    m_SelectedShadowIndex = -1;
    m_pSelectedShadow = nullptr;
}
