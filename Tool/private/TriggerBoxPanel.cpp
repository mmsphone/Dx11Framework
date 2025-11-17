#include "TriggerBoxPanel.h"

#include "EngineUtility.h"

#include "TriggerBox.h"

TriggerBoxPanel::TriggerBoxPanel(const std::string& panelName, bool open)
    : Panel{ panelName, open }
{
}

HRESULT TriggerBoxPanel::Initialize()
{
    // 패널 기본 위치/크기
    m_PanelPosition = _float2(900.f, 200.f);
    m_PanelSize = _float2(400.f, 400.f);

    m_SelectedTriggerIndex = -1;
    m_pSelectedTrigger = nullptr;

    m_EditCenter = XMFLOAT3(0.f, 0.f, 0.f);
    m_EditExtents = XMFLOAT3(1.f, 1.f, 1.f);

    return S_OK;
}

void TriggerBoxPanel::OnRender()
{
    ImGui::Text("Trigger Box Panel");
    ImGui::Separator();

    // ------------------------------------------------------
    // 1) 현재 TriggerBox 리스트
    // ------------------------------------------------------
    vector<TriggerBox*> triggers = m_pEngineUtility->GetTriggerBoxes();
    const int triggerCount = static_cast<int>(triggers.size());

    ImGui::Text("Triggers (%d)", triggerCount);

    ImGui::BeginChild("TriggerBoxList", ImVec2(0, 150), true, ImGuiWindowFlags_HorizontalScrollbar);

    int idx = 0;
    TriggerBox* clickedTrigger = nullptr;

    for (auto* pTrigger : triggers)
    {
        char label[64];
        sprintf_s(label, "Trigger_%d", idx);

        bool selected = (idx == m_SelectedTriggerIndex && pTrigger == m_pSelectedTrigger);

        if (ImGui::Selectable(label, selected))
        {
            m_SelectedTriggerIndex = idx;
            m_pSelectedTrigger = pTrigger;
            clickedTrigger = pTrigger;
        }
        ++idx;
    }

    ImGui::EndChild();

    // 선택 변경 시, 내부 편집 값 동기화
    if (clickedTrigger)
        SyncFromSelectedTriggerBox();

    ImGui::Separator();

    // ------------------------------------------------------
    // 2) 선택된 트리거 정보 표시 / 삭제
    // ------------------------------------------------------
    if (m_pSelectedTrigger && m_SelectedTriggerIndex >= 0 && m_SelectedTriggerIndex < triggerCount)
    {
        ImGui::Text("Selected Trigger: %d", m_SelectedTriggerIndex);

        // 현재 TriggerBox의 center/extents 표시용 (읽기 전용으로 Text만)
        ImGui::DragFloat3("Center##EditTrigger", &m_EditCenter.x, 0.1f);
        ImGui::DragFloat3("Extents##EditTrigger", &m_EditExtents.x, 0.1f, 0.f, 10000.f);

        if (ImGui::Button("Apply##TriggerBox"))
        {
            ApplyToSelectedTriggerBox();
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Trigger"))
        {
            if (FAILED(m_pEngineUtility->RemoveTriggerBox(static_cast<_uint>(m_SelectedTriggerIndex))))
            {
                MessageBoxA(nullptr, "RemoveTriggerBox failed.", "TriggerBox", MB_OK);
            }
            else
            {
                m_SelectedTriggerIndex = -1;
                m_pSelectedTrigger = nullptr;
            }
        }
    }
    else
    {
        ImGui::TextDisabled("No trigger selected.");
    }

    ImGui::Separator();
    ImGui::Text("Create New Trigger Box");

    // ------------------------------------------------------
    // 3) 새 트리거 생성 (수동 입력)
    // ------------------------------------------------------
    static XMFLOAT3 s_newCenter = XMFLOAT3(0.f, 0.f, 0.f);
    static XMFLOAT3 s_newExtents = XMFLOAT3(1.f, 1.f, 1.f);

    ImGui::DragFloat3("Center##NewTrigger", &s_newCenter.x, 0.1f);
    ImGui::DragFloat3("Extents##NewTrigger", &s_newExtents.x, 0.1f, 0.0f, 10000.f);

    if (ImGui::Button("Add TriggerBox (Manual)"))
    {
        TRIGGERBOX_DESC desc{};
        desc.center = _float3{ s_newCenter.x,  s_newCenter.y,  s_newCenter.z };
        desc.Extents = _float3{ s_newExtents.x, s_newExtents.y, s_newExtents.z };

        TriggerBox* pTriggerBox = TriggerBox::Create(&desc);
        if (FAILED(m_pEngineUtility->AddTriggerBox(pTriggerBox)))
        {
            MessageBoxA(nullptr, "AddTriggerBox failed.", "TriggerBox", MB_OK);
        }
    }

    ImGui::Separator();

    // ------------------------------------------------------
    // 4) Save / Load 트리거 데이터
    // ------------------------------------------------------
    if (ImGui::Button("Save TriggerBoxes..."))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Trigger Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

        if (GetSaveFileNameA(&ofn))
        {
            if (FAILED(m_pEngineUtility->SaveTriggerBoxes(szFile)))
            {
                MessageBoxA(nullptr, "SaveTriggerBoxes failed.", "TriggerBox", MB_OK);
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load TriggerBoxes..."))
    {
        char szFile[MAX_PATH] = {};
        OPENFILENAMEA ofn = {};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Trigger Data (*.dat)\0*.dat\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameA(&ofn))
        {
            if (FAILED(m_pEngineUtility->LoadTriggerBoxes(szFile)))
            {
                MessageBoxA(nullptr, "ReadyTriggerBoxesFromFile failed.", "TriggerBox", MB_OK);
            }

            // 로드 후 선택 초기화
            m_SelectedTriggerIndex = -1;
            m_pSelectedTrigger = nullptr;
        }
    }
}

TriggerBoxPanel* TriggerBoxPanel::Create(const std::string& panelName, bool open)
{
    TriggerBoxPanel* pInstance = new TriggerBoxPanel(panelName, open);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : TriggerBoxPanel");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void TriggerBoxPanel::Free()
{
    __super::Free();
}

void TriggerBoxPanel::SyncFromSelectedTriggerBox()
{
    if (!m_pSelectedTrigger)
        return;

    TRIGGERBOX_DESC desc = m_pSelectedTrigger->GetTriggerBoxDesc();

    m_EditCenter = desc.center;
    m_EditExtents = desc.Extents;
}

void TriggerBoxPanel::ApplyToSelectedTriggerBox()
{
    if (!m_pSelectedTrigger)
        return;

    TRIGGERBOX_DESC desc{};
    desc.center = _float3(m_EditCenter.x, m_EditCenter.y, m_EditCenter.z);
    desc.Extents = _float3(m_EditExtents.x, m_EditExtents.y, m_EditExtents.z);

    m_pSelectedTrigger->UpdateFromDesc(desc);
}
