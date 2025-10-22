#include "ObjectPanel.h"

#include "EngineUtility.h"
#include "Object.h"
#include "ImGuizmo.h"

ObjectPanel::ObjectPanel(const string& PanelName, bool open)
	:Panel{ PanelName, open }
{
}

HRESULT ObjectPanel::Initialize(Object* pTarget)
{
    if (pTarget == nullptr)
        return E_FAIL;
    m_pTargetObject = pTarget;
    SafeAddRef(m_pTargetObject);

    m_PanelPosition = _float2(400.f, 500.f);
    m_PanelSize = _float2(400.f, 200.f);

    return S_OK;
}

void ObjectPanel::OnRender()
{
    ImGui::SeparatorText("Gizmo Mode");
    static ImGuizmo::OPERATION gizmoOp = ImGuizmo::TRANSLATE;

    if (ImGui::RadioButton("Translate", gizmoOp == ImGuizmo::TRANSLATE)) gizmoOp = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", gizmoOp == ImGuizmo::ROTATE)) gizmoOp = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", gizmoOp == ImGuizmo::SCALE)) gizmoOp = ImGuizmo::SCALE;

    m_pEngineUtility->SetGizmoState(m_pTargetObject, gizmoOp);

    if (ImGui::Button("Delete Object"))
    {
        SetOpen(false);
        m_pTargetObject->SetDead(true);
    }
}

ObjectPanel* ObjectPanel::Create(const string& PanelName, bool open, Object* pTarget)
{
    ObjectPanel* pInstance = new ObjectPanel(PanelName, open);

    if (FAILED(pInstance->Initialize(pTarget)))
    {
        MSG_BOX("Failed to Created : ObjectPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void ObjectPanel::Free()
{
    SafeRelease(m_pTargetObject);
}
