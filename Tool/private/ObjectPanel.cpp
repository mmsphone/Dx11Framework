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
    m_PanelSize = _float2(400.f, 400.f);

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

    // --------------------------
    // 🔹 Transform 수치 직접 입력 UI
    // --------------------------
    Transform* pTransform = dynamic_cast<Transform*>(m_pTargetObject->FindComponent(TEXT("Transform")));
    if (pTransform)
    {
        _float4x4 matWorld = *pTransform->GetWorldMatrixPtr();

        // Position / Rotation / Scale 분해
        XMVECTOR S, R, T;
        XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&matWorld));

        XMFLOAT3 pos, scl;
        XMStoreFloat3(&pos, T);
        XMStoreFloat3(&scl, S);

        // 쿼터니언을 오일러로 변환
        XMFLOAT4 quat;
        XMStoreFloat4(&quat, R);
        XMFLOAT3 rot = {};
        {
            XMVECTOR q = XMLoadFloat4(&quat);
            XMFLOAT4 Q;
            XMStoreFloat4(&Q, q);

            float sinp = 2.0f * (Q.w * Q.x + Q.y * Q.z);
            float cosp = 1.0f - 2.0f * (Q.x * Q.x + Q.y * Q.y);
            rot.x = XMConvertToDegrees(std::atan2(sinp, cosp));

            float siny = 2.0f * (Q.w * Q.y - Q.z * Q.x);
            siny = std::clamp(siny, -1.0f, 1.0f);
            rot.y = XMConvertToDegrees(std::asin(siny));

            float sinr = 2.0f * (Q.w * Q.z + Q.x * Q.y);
            float cosr = 1.0f - 2.0f * (Q.y * Q.y + Q.z * Q.z);
            rot.z = XMConvertToDegrees(std::atan2(sinr, cosr));
        }

        ImGui::Text("Transform (Direct Edit)");
        bool changed = false;

        changed |= ImGui::DragFloat3("Position", (float*)&pos, 0.01f, -10000.f, 10000.f);
        changed |= ImGui::DragFloat3("Rotation", (float*)&rot, 0.1f, -360.f, 360.f);
        changed |= ImGui::DragFloat3("Scale", (float*)&scl, 0.01f, 0.0f, 1000.f);
        
        if (ImGui::Button("Reset Transform"))
        {
            pos = { 0.f, 0.f, 0.f };
            rot = { 0.f, 0.f, 0.f };
            scl = { 1.f, 1.f, 1.f };
            changed = true;        
        }

        if(changed)
        {
            // 다시 행렬 재구성
            XMMATRIX matS = XMMatrixScaling(scl.x, scl.y, scl.z);
            XMMATRIX matR = XMMatrixRotationRollPitchYaw(
                XMConvertToRadians(rot.x),
                XMConvertToRadians(rot.y),
                XMConvertToRadians(rot.z));
            XMMATRIX matT = XMMatrixTranslation(pos.x, pos.y, pos.z);

            XMMATRIX newWorld = matS * matR * matT;

            // RIGHT / UP / LOOK / POSITION 추출
            _vector vRight = newWorld.r[0];
            _vector vUp = newWorld.r[1];
            _vector vLook = newWorld.r[2];
            _vector vPos = newWorld.r[3];

            // Transform에 반영
            pTransform->SetState(STATE::RIGHT, vRight);
            pTransform->SetState(STATE::UP, vUp);
            pTransform->SetState(STATE::LOOK, vLook);
            pTransform->SetState(STATE::POSITION, vPos);
        }

        ImGui::Separator();
    }

    // --------------------------
    // 🔹 Animation 컨트롤
    // --------------------------
    Model* pModel = dynamic_cast<Model*>(m_pTargetObject->FindComponent(TEXT("Model")));
    if (pModel)
    {
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

        // 🔹 애니메이션 진행도 표시
        _float cur = pModel->GetCurAnimTrackPos();
        _float dur = pModel->GetCurAnimDuration();
        if (dur > 0.f)
        {
            float progress = cur / dur;
            ImGui::ProgressBar(progress, ImVec2(-FLT_MIN, 0), "Anim Progress");
        }

        ImGui::Separator();
    }

    // --------------------------
    // 🔹 삭제 버튼
    // --------------------------
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
