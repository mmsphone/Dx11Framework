#include "hacking2GameUI.h"

#include "EngineUtility.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "Console.h"
#include "Player.h"
#include "Layer.h"

hacking2GameUI::hacking2GameUI()
    : UI{}
{
}

hacking2GameUI::hacking2GameUI(const hacking2GameUI& Prototype)
    : UI{ Prototype }
{
}

HRESULT hacking2GameUI::Initialize(void* pArg)
{
    UI_DESC desc{};
    desc.fRotationPerSec = 0.f;
    desc.fSpeedPerSec = 0.f;
    desc.name = "hackingGameUI";
    desc.visible = false;
    desc.enable = false;
    if (FAILED(__super::Initialize(&desc)))
        return E_FAIL;

    m_active = false;
    m_minimized = false;
    m_finished = false;
    m_doneProgress = 0.f;

    if (FAILED(SetupButtons()))
        return E_FAIL;

    if (FAILED(SetupNumberLabels()))
        return E_FAIL;

    InitGame();

    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(true);

    return S_OK;
}

void hacking2GameUI::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_minimized)
        return;

    if (m_active)
    {
        UpdateLines(fTimeDelta);

        UpdateAllLineButtonVisual();

        if (IsAllLinesAligned())
            OnGameClear();
    }

    if (m_finished)
    {
        UpdateDoneEffect(fTimeDelta);
    }
}

void hacking2GameUI::SetConsole(Console* pConsole)
{
    m_pConsole = pConsole;
}

void hacking2GameUI::ShowGame()
{
    m_minimized = false;
    if (m_finished)
    {
        m_pEngineUtility->PlaySound2D("BGM_minigame2downloading");
        SetDoneUIVisible(true);
    }
    else
    {
        m_pEngineUtility->PlaySound2D("FBX_minigame2show");
        SetWindowVisible(true);
        UpdateAllLineButtonVisual();
        UpdateAllLineNumbers();
    }
}

void hacking2GameUI::MinimizeGame()
{
    if (m_finished)
        m_pEngineUtility->StopSound("BGM_minigame2downloading");

    m_minimized = true;
    SetWindowVisible(false);
    SetDoneUIVisible(false);
}

void hacking2GameUI::CloseGame()
{
    if(m_finished)
        m_pEngineUtility->StopSound("BGM_minigame2downloading");

    m_active = false;
    m_minimized = false;
    m_finished = false;

    SetWindowVisible(false);
    SetDoneUIVisible(false);

    m_doneProgress = 0.f;

    if (!m_pDoneProgressBar)
        m_pDoneProgressBar = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"hacking2_done_progressbar"));
    if (m_pDoneProgressBar)
        m_pDoneProgressBar->SetLoadingRatio(0.f);

    m_pEngineUtility->RemoveUI(L"hacking2GameUI");
    SetDead(true);
}

HRESULT hacking2GameUI::SetupButtons()
{
    // Close 버튼
    if (UIButton* pClose = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking2_close")))
    {
        pClose->SetEnable(true);
        pClose->ClearButtonFunctions();
        pClose->AddLeftButtonFunction([this]()
            {
                CloseGame();
            });
        pClose->SetDefaultImage(L"hacking2_close_default");
        pClose->SetOnImage(L"hacking2_close_on");
    }

    // Minimize 버튼
    if (UIButton* pMinimize = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking2_minimize")))
    {
        pMinimize->SetEnable(true);
        pMinimize->ClearButtonFunctions();
        pMinimize->AddLeftButtonFunction([this]()
            {
                MinimizeGame();
            });
        pMinimize->SetDefaultImage(L"hacking2_minimize_default");
        pMinimize->SetOnImage(L"hacking2_minimize_on");
    }

    // Up / Down 버튼 4세트
    const wchar_t* upBtnNames[LINE_COUNT] =
    {
        L"hacking2_updown1_up",
        L"hacking2_updown2_up",
        L"hacking2_updown3_up",
        L"hacking2_updown4_up"
    };
    const wchar_t* downBtnNames[LINE_COUNT] =
    {
        L"hacking2_updown1_down",
        L"hacking2_updown2_down",
        L"hacking2_updown3_down",
        L"hacking2_updown4_down"
    };

    for (_uint i = 0; i < LINE_COUNT; ++i)
    {
        // Up 버튼
        UIButton* pUp = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(upBtnNames[i]));
        m_controls[i].pUp = pUp;
        if (pUp)
        {
            pUp->SetEnable(true);
            pUp->ClearButtonFunctions();
            pUp->AddLeftButtonFunction([this, i]()
                {
                    OnClickDir(i, -1); // 위 방향
                });
        }

        // Down 버튼
        UIButton* pDown = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(downBtnNames[i]));
        m_controls[i].pDown = pDown;
        if (pDown)
        {
            pDown->SetEnable(true);
            pDown->ClearButtonFunctions();
            pDown->AddLeftButtonFunction([this, i]()
                {
                    OnClickDir(i, +1); // 아래 방향
                });
        }

        m_lines[i].currentRow = (_int)i;
        m_lines[i].dir = -1;   // 기본은 위 방향
        m_lines[i].acc = 0.f;
    }

    // done 프로그레스 바 캐싱
    m_pDoneProgressBar = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"hacking2_done_progressbar"));
    if (m_pDoneProgressBar)
        m_pDoneProgressBar->SetLoadingRatio(0.f);

    return S_OK;
}

HRESULT hacking2GameUI::SetupNumberLabels()
{
    const float baseX = 469.f;   // 1번 라인 X
    const float baseY = 292.f;   // row 0 Y
    const float lineGap = 33.f;  // 라인 사이 X 간격
    const float rowGap = 24.f;  // row 사이 Y 간격

    for (_uint line = 0; line < LINE_COUNT; ++line)
    {
        for (_uint row = 0; row < ROW_COUNT; ++row)
        {
            UI_DESC desc{};
            desc.fSpeedPerSec = 0.f;
            desc.fRotationPerSec = 0.f;
            desc.type = UITYPE::UI_LABEL;
            desc.visible = false;  // 창 열릴 때 SetWindowVisible에서 켜줄 것
            desc.enable = false;

            // 이름 규칙: hacking2_line{line}_row{row}
            char nameBuf[64]{};
            sprintf_s(nameBuf, "hacking2_line%u_row%u", line, row);
            desc.name = nameBuf;

            desc.x = baseX + lineGap * line;
            desc.y = baseY + rowGap * row;
            desc.z = 0.f;
            desc.w = 20.f;   // 폰트 폭 (적당히)
            desc.h = 20.f;   // 폰트 높이
            desc.fontSize = 20.f;
            desc.fontColor = _float4{ 0.4f, 0.4f, 0.4f,0.3f };

            // UILabel 객체 생성 (STATIC 씬의 "UILabel" 프로토타입 사용, GAMEPLAY 씬 UI 레이어에 배치)
            m_pEngineUtility->AddObject(
                SCENE::STATIC,
                L"UILabel",
                SCENE::GAMEPLAY,
                L"UI",
                &desc);

            // UI 레이어에서 마지막으로 추가된 객체를 UILabel로 캐스팅해서 저장
            auto& objs = m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects();
            if (objs.empty())
                return E_FAIL;

            UILabel* pLabel = dynamic_cast<UILabel*>(objs.back());
            if (!pLabel)
                return E_FAIL;

            pLabel->SetText(L"0"); // 초기값은 0
            m_numberLabels[line][row] = pLabel;
        }
    }

    return S_OK;
}


void hacking2GameUI::InitGame()
{
    m_active = true;
    m_finished = false;
    m_minimized = false;

    for (_uint i = 0; i < LINE_COUNT; ++i)
    {
        m_lines[i].currentRow = (_int)i;
        m_lines[i].acc = 0.f;
        m_lines[i].dir = m_pEngineUtility->Random(-1,1) >= 0.f ? 1 : -1;
    }

    m_doneProgress = 0.f;
    if (m_pDoneProgressBar)
        m_pDoneProgressBar->SetLoadingRatio(0.f);

    SetDoneUIVisible(false);
    SetWindowVisible(true);

    UpdateAllLineButtonVisual();
    UpdateAllLineNumbers();
}

void hacking2GameUI::UpdateLines(_float fTimeDelta)
{
    _bool anyMoved = false;

    for (_uint i = 0; i < LINE_COUNT; ++i)
    {
        LineState& line = m_lines[i];

        line.acc += fTimeDelta;
        if (line.acc < m_scrollInterval)
            continue;

        line.acc -= m_scrollInterval;
        anyMoved = true;

        line.currentRow += line.dir;

        if (line.currentRow < 0)
            line.currentRow += (_int)ROW_COUNT;
        else if (line.currentRow >= (_int)ROW_COUNT)
            line.currentRow -= (_int)ROW_COUNT;
    }

    // 한 칸이라도 움직였으면 숫자판 갱신
    if (anyMoved)
        UpdateAllLineNumbers();
}

void hacking2GameUI::UpdateDoneEffect(_float fTimeDelta)
{
    if (m_pDoneProgressBar)
    {
        m_doneProgress += m_doneFillSpeed * fTimeDelta;
        if (m_doneProgress > 1.f)
            m_doneProgress = 1.f;

        m_pDoneProgressBar->SetLoadingRatio(m_doneProgress);
    }

    UI* pLeft = m_pEngineUtility->FindUI(L"hacking2_done_left");
    UI* pRight = m_pEngineUtility->FindUI(L"hacking2_done_right");
    UI* pMove = m_pEngineUtility->FindUI(L"hacking2_done_move");

    UI_DESC desc = pMove->GetUIDesc();
    _float startPos = pLeft->GetUIDesc().x + pLeft->GetUIDesc().w;
    _float goalPos = pRight->GetUIDesc().x - pRight->GetUIDesc().w;

    const _float travelCount = 5.f;               // 좌->우 횟수
    _float tScaled = m_doneProgress * travelCount; // 0 ~ 5
    _float alpha = tScaled - floorf(tScaled);    // 0 ~ 1 (각 사이클 내 진행도)

    // alpha: 0일 때 왼쪽, 1일 때 오른쪽
    desc.x = startPos + (goalPos - startPos) * alpha;
    pMove->ApplyUIDesc(desc);

    if (m_doneProgress >= 1.f)
    {
        {
            Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
            QUEST_EVENT ev{};
            ev.type = EVENTTYPE_INTERACT;
            ev.pInstigator = pPlayer;
            ev.pTarget = this;
            ev.tag = L"console1";
            m_pEngineUtility->PushEvent(ev);
        }

        CloseGame();
    }
}

void hacking2GameUI::OnClickDir(_uint index, _int dir)
{
    if (index >= LINE_COUNT)
        return;

    m_lines[index].dir = dir;
    UpdateLineButtonVisual(index);

    m_pEngineUtility->PlaySound2D("FBX_minigame2Button");
}

_bool hacking2GameUI::IsAllLinesAligned() const
{
    // 기준: 0번 라인의 위치 + 방향
    const _int targetRow = m_lines[0].currentRow;
    const _int targetDir = m_lines[0].dir;

    for (_uint i = 1; i < LINE_COUNT; ++i)
    {
        if (m_lines[i].currentRow != targetRow)
            return false;

        if (m_lines[i].dir != targetDir)
            return false;
    }

    return true;
}

void hacking2GameUI::SetWindowVisible(_bool visible)
{
    // 게임창 전체 구성요소들 (배경/판)
    const wchar_t* baseNames[] =
    {
        L"hacking2_backplate",
        L"hacking2_gameplate",
        L"hacking2_access_back",
        L"hacking2_updown1_back",
        L"hacking2_updown2_back",
        L"hacking2_updown3_back",
        L"hacking2_updown4_back",

        // close / minimize 기본 이미지
        L"hacking2_close_default",
        L"hacking2_minimize_default",
    };

    for (const wchar_t* name : baseNames)
    {
        if (UI* p = m_pEngineUtility->FindUI(name))
            p->SetVisible(visible);
    }

    // ★ up/down 기본 통짜(default) 이미지는 창이 켜지면 일단 다 켜두고
    // 어떤 걸 살릴지는 UpdateLineButtonVisual에서 dir 기준으로 정리
    const wchar_t* defaultImgs[] =
    {
        L"hacking2_updown1_up_default",
        L"hacking2_updown1_down_default",
        L"hacking2_updown2_up_default",
        L"hacking2_updown2_down_default",
        L"hacking2_updown3_up_default",
        L"hacking2_updown3_down_default",
        L"hacking2_updown4_up_default",
        L"hacking2_updown4_down_default",
    };

    for (const wchar_t* name : defaultImgs)
    {
        if (UI* p = m_pEngineUtility->FindUI(name))
            p->SetVisible(visible);
    }

    // ★ on 오버레이는 여기서는 항상 끈다. (hover/dir에 따라 나중에 켬)
    const wchar_t* overlayImgs[] =
    {
        L"hacking2_updown1_up_on",
        L"hacking2_updown1_down_on",
        L"hacking2_updown2_up_on",
        L"hacking2_updown2_down_on",
        L"hacking2_updown3_up_on",
        L"hacking2_updown3_down_on",
        L"hacking2_updown4_up_on",
        L"hacking2_updown4_down_on",
        L"hacking2_close_on",
        L"hacking2_minimize_on",
    };

    for (const wchar_t* name : overlayImgs)
    {
        if (UI* p = m_pEngineUtility->FindUI(name))
            p->SetVisible(false);
    }

    // 버튼 Enable
    const wchar_t* upBtns[LINE_COUNT] =
    {
        L"hacking2_updown1_up",
        L"hacking2_updown2_up",
        L"hacking2_updown3_up",
        L"hacking2_updown4_up"
    };
    const wchar_t* downBtns[LINE_COUNT] =
    {
        L"hacking2_updown1_down",
        L"hacking2_updown2_down",
        L"hacking2_updown3_down",
        L"hacking2_updown4_down"
    };

    for (_uint i = 0; i < LINE_COUNT; ++i)
    {
        if (UIButton* p = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(upBtns[i])))
            p->SetEnable(visible);
        if (UIButton* p = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(downBtns[i])))
            p->SetEnable(visible);
    }

    if (UIButton* p = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking2_close")))
        p->SetEnable(visible);
    if (UIButton* p = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking2_minimize")))
        p->SetEnable(visible);

    for (_uint line = 0; line < LINE_COUNT; ++line)
    {
        for (_uint row = 0; row < ROW_COUNT; ++row)
        {
            UILabel* pLabel = m_numberLabels[line][row];
            if (pLabel)
                pLabel->SetVisible(visible);
        }
    }

    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(visible);
}

void hacking2GameUI::SetDoneUIVisible(_bool visible)
{
    if (visible == true)
    {
        const wchar_t* baseNames[] =
        {
            L"hacking2_backplate",
            L"hacking2_gameplate",
            L"hacking2_close_default",
            L"hacking2_minimize_default",
        };
        for (const wchar_t* name : baseNames)
        {
            if (UI* p = m_pEngineUtility->FindUI(name))
                p->SetVisible(true);
        }

        UIButton* pClose = static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking2_close"));
        pClose->SetEnable(true);
        UIButton* pMinimize = static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking2_minimize"));
        pMinimize->SetEnable(true);
    }
   
    const wchar_t* names[] =
    {
        L"hacking2_done_left",
        L"hacking2_done_move",
        L"hacking2_done_right",
        L"hacking2_done_progress",
        L"hacking2_done_progressbar",
    };

    for (const wchar_t* name : names)
    {
        if (UI* p = m_pEngineUtility->FindUI(name))
            p->SetVisible(visible);
    }
}

void hacking2GameUI::OnGameClear()
{
    m_active = false;
    m_finished = true;
    m_pConsole->UnlockConsole();
    m_pConsole->SetVisibleConsoleUI(true);

    SetWindowVisible(false);
    SetDoneUIVisible(true);

    m_doneProgress = 0.f;
    if (!m_pDoneProgressBar)
        m_pDoneProgressBar = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"hacking2_done_progressbar"));
    if (m_pDoneProgressBar)
        m_pDoneProgressBar->SetLoadingRatio(0.f);

    {
        Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
        QUEST_EVENT ev{};
        ev.type = EVENTTYPE_INTERACT;
        ev.pInstigator = pPlayer;
        ev.pTarget = this;
        ev.tag = L"console0";
        m_pEngineUtility->PushEvent(ev);
    }
    m_pEngineUtility->PlaySound2D("BGM_minigame2downloading");

    _float r = m_pEngineUtility->Random(0, 3);
    if (r >= 2)
        m_pEngineUtility->PlaySound2D("FBX_playerHackComplete1");
    else if (r >= 1)
        m_pEngineUtility->PlaySound2D("FBX_playerHackComplete2");
    else
        m_pEngineUtility->PlaySound2D("FBX_playerHackComplete3");
}

void hacking2GameUI::UpdateLineButtonVisual(_uint index)
{
    if (index >= LINE_COUNT)
        return;

    const _int dir = m_lines[index].dir;

    static const wchar_t* upDef[LINE_COUNT] =
    {
        L"hacking2_updown1_up_default",
        L"hacking2_updown2_up_default",
        L"hacking2_updown3_up_default",
        L"hacking2_updown4_up_default"
    };
    static const wchar_t* upOn[LINE_COUNT] =
    {
        L"hacking2_updown1_up_on",
        L"hacking2_updown2_up_on",
        L"hacking2_updown3_up_on",
        L"hacking2_updown4_up_on"
    };
    static const wchar_t* downDef[LINE_COUNT] =
    {
        L"hacking2_updown1_down_default",
        L"hacking2_updown2_down_default",
        L"hacking2_updown3_down_default",
        L"hacking2_updown4_down_default"
    };
    static const wchar_t* downOn[LINE_COUNT] =
    {
        L"hacking2_updown1_down_on",
        L"hacking2_updown2_down_on",
        L"hacking2_updown3_down_on",
        L"hacking2_updown4_down_on"
    };

    UI* pUpDef = m_pEngineUtility->FindUI(upDef[index]);
    UI* pUpOn = m_pEngineUtility->FindUI(upOn[index]);
    UI* pDownDef = m_pEngineUtility->FindUI(downDef[index]);
    UI* pDownOn = m_pEngineUtility->FindUI(downOn[index]);

    // 1) 우선 오버레이는 다 끄고 시작
    if (pUpOn)    pUpOn->SetVisible(false);
    if (pDownOn)  pDownOn->SetVisible(false);

    // 2) 현재 방향에 맞는 default 하나만 켠다
    if (dir <= 0)
    {
        // 방향: 위
        if (pUpDef)   pUpDef->SetVisible(true);
        if (pDownDef) pDownDef->SetVisible(false);
    }
    else
    {
        // 방향: 아래
        if (pUpDef)   pUpDef->SetVisible(false);
        if (pDownDef) pDownDef->SetVisible(true);
    }

    // 3) 마우스 오버 상태에 따라 on 오버레이만 얹기
    const _bool overUp = (m_controls[index].pUp && m_controls[index].pUp->IsMouseOver());
    const _bool overDown = (m_controls[index].pDown && m_controls[index].pDown->IsMouseOver());

    if (overUp && pUpOn)
    {
        pUpOn->SetVisible(true);
    }
    else if (overDown && pDownOn)
    {
        pDownOn->SetVisible(true);
    }
}

void hacking2GameUI::UpdateAllLineButtonVisual()
{
    for (_uint i = 0; i < LINE_COUNT; ++i)
        UpdateLineButtonVisual(i);
}

void hacking2GameUI::UpdateLineNumbers(_uint lineIndex)
{
    if (lineIndex >= LINE_COUNT)
        return;

    const _int oneRow = m_lines[lineIndex].currentRow;

    for (_uint row = 0; row < ROW_COUNT; ++row)
    {
        UILabel* pLabel = m_numberLabels[lineIndex][row];
        if (!pLabel)
            continue;

        if ((_int)row == oneRow)
        {
            pLabel->SetText(L"1");
            pLabel->SetColor(_vector{ 1.f,1.f,1.f,1.f });
        }
        else
        {
            pLabel->SetText(L"0");
            pLabel->SetColor(_vector{ 0.4f,0.4f,0.4f,0.3f });
        }
    }
}

void hacking2GameUI::UpdateAllLineNumbers()
{
    for (_uint i = 0; i < LINE_COUNT; ++i)
        UpdateLineNumbers(i);
}

hacking2GameUI* hacking2GameUI::Create()
{
    hacking2GameUI* pInstance = new hacking2GameUI();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : hacking2GameUI");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* hacking2GameUI::Clone(void* pArg)
{
    hacking2GameUI* pInstance = new hacking2GameUI(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : hacking2GameUI");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void hacking2GameUI::Free()
{
    __super::Free();
}
