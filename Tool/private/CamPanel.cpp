#include "CamPanel.h"

#include "Tool_Defines.h"

#include "EngineUtility.h"
#include "Object.h"

CamPanel::CamPanel(const string& PanelName, bool open)
	:Panel{PanelName, open}
{
}

HRESULT CamPanel::Initialize(SCENE eType)
{
    m_pCamObject = m_pEngineUtility->FindObject(eType, TEXT("Cam"), 0);
    if (m_pCamObject == nullptr)
        return E_FAIL;
    SafeAddRef(m_pCamObject);

    m_SceneType = eType;

    m_PanelPosition = _float2(400.f, 50.f);
    m_PanelSize = _float2(400.f, 800.f);

    return S_OK;
}

void CamPanel::OnRender()
{
    if (m_pCamObject == nullptr) return;

    Transform* pTransform = dynamic_cast<Transform*>(m_pCamObject->FindComponent(TEXT("Transform")));
    if (pTransform == nullptr) return;

    _float3 vPos{};
    XMStoreFloat3(&vPos, pTransform->GetState(POSITION));
    ImGui::Text("--Cam Position--");
    ImGui::Text("X : %f / Y : %f / Z : %f", vPos.x, vPos.y, vPos.z);
    ImGui::Text("\n");

    _float3 vDir{};
    XMStoreFloat3(&vDir, pTransform->GetState(LOOK));
    ImGui::Text("--Cam Forward Direction--");
    ImGui::Text("X : %f / Y : %f / Z : %f", vDir.x, vDir.y, vDir.z);
    ImGui::Text("\n");

    static _float2 vSpeed{
    vSpeed.x = pTransform->GetSpeedPerSec(),
    vSpeed.y = pTransform->GetRotatePerSec()
    };
    ImGui::InputFloat("Move Speed", &vSpeed.x);
    ImGui::InputFloat("Rotate Speed", &vSpeed.y);

    pTransform->SetSpeedPerSec(vSpeed.x);
    pTransform->SetRotatePerSec(vSpeed.y);
}


CamPanel* CamPanel::Create(const string& PanelName, SCENE eType, bool open)
{
    CamPanel* pInstance = new CamPanel(PanelName, open);

    if (FAILED(pInstance->Initialize(eType)))
    {
        MSG_BOX("Failed to Created : CamPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void CamPanel::Free()
{
    __super::Free();
    SafeRelease(m_pCamObject);
}