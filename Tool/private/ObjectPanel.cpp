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

    m_PanelPosition = _float2(900.f, 300.f);
    m_PanelSize = _float2(400.f, 200.f);

    return S_OK;
}

void ObjectPanel::OnRender()
{
    static _bool bActiveGuizmo = true;
    ImGui::SeparatorText("Gizmo Mode");
    static ImGuizmo::OPERATION gizmoOp = ImGuizmo::TRANSLATE;

    if (ImGui::RadioButton("None", !bActiveGuizmo))
        bActiveGuizmo = false;

    ImGui::SameLine();
    if (ImGui::RadioButton("Translate", bActiveGuizmo && gizmoOp == ImGuizmo::TRANSLATE))
    {
        gizmoOp = ImGuizmo::TRANSLATE;
        bActiveGuizmo = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", bActiveGuizmo && gizmoOp == ImGuizmo::ROTATE))
    {
        gizmoOp = ImGuizmo::ROTATE;
        bActiveGuizmo = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", bActiveGuizmo && gizmoOp == ImGuizmo::SCALE))
    {
        gizmoOp = ImGuizmo::SCALE;
        bActiveGuizmo = true;
    }

    if (bActiveGuizmo)
        m_pEngineUtility->SetGizmoState(m_pTargetObject, gizmoOp);
    else
        m_pEngineUtility->ClearGizmoState();
    ImGui::Separator();

    Model* pModel = dynamic_cast<Model*>(m_pTargetObject->FindComponent(TEXT("Model")));
    static _int iTargetAnimIndex = 0;
    ImGui::Text("--TargetAnimIndex(Default : 0)--");
    if (ImGui::InputInt("##TargetMeshIndex", &iTargetAnimIndex))
    {
        pModel->SetAnimation(iTargetAnimIndex, true);
    }
    if (ImGui::Button("PlayAnim"))
    {
        pModel->ResumeAnimation();
    }
    ImGui::SameLine();
    if (ImGui::Button("StopAnim"))
    {
        pModel->StopAnimation();
    }
    ImGui::Separator();
    
    if (ImGui::Button("Delete Object"))
    {
        SetOpen(false);
        m_pEngineUtility->ClearGizmoState();
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
