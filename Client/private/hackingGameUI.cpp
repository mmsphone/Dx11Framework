#include "hackingGameUI.h"

#include "EngineUtility.h"
#include "Panel.h"
#include "UIButton.h"
#include "UIImage.h"
#include "Layer.h"
#include "Player.h"

namespace
{
    // 방향 비트 인덱스
    enum DIR_BIT
    {
        DIR_UP = 0,
        DIR_RIGHT = 1,
        DIR_DOWN = 2,
        DIR_LEFT = 3,
    };

    inline unsigned char MakeMask(bool up, bool right, bool down, bool left)
    {
        return (up ? (1 << DIR_UP) : 0)
            | (right ? (1 << DIR_RIGHT) : 0)
            | (down ? (1 << DIR_DOWN) : 0)
            | (left ? (1 << DIR_LEFT) : 0);
    }
}

hackingGameUI::hackingGameUI()
	:UI{}
{
}

hackingGameUI::hackingGameUI(const hackingGameUI& Prototype)
	:UI{Prototype}
{
}

HRESULT hackingGameUI::Initialize(void* pArg)
{
    UI_DESC desc{};
    desc.fRotationPerSec = 0.f;
    desc.fSpeedPerSec = 0.f;
    desc.name = "hackingGameUI";
    desc.visible = false;
    desc.enable = false;
    if(FAILED(__super::Initialize(&desc)))
        return E_FAIL;

    m_pPanel = nullptr;
    m_finished = false;
    m_lineSolved[0] = m_lineSolved[1] = false;

    m_progressCur = 0.f;
    m_progressTarget = 0.f;
    m_allSolved = false;
    m_clearWaitAcc = 0.f;

    m_timeLimit = 10.f;
    m_timeAcc = 0.f;
    m_fastStartX = 0.f;
    m_fastEndX = 0.f;

	m_pEngineUtility->FindUI(L"hacking_back")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_gameplate")->SetVisible(true);

	m_pEngineUtility->FindUI(L"hacking_done1")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_done2")->SetVisible(true);

	m_pEngineUtility->FindUI(L"hacking_start1")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_start2")->SetVisible(true);

	m_pEngineUtility->FindUI(L"hacking_goal1")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_goal2")->SetVisible(true);

	m_pEngineUtility->FindUI(L"hacking_back_text")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_gameplate_text")->SetVisible(true);

	m_pEngineUtility->FindUI(L"hacking_progress")->SetVisible(true);
    m_pProgressBar = static_cast<UIImage*>(m_pEngineUtility->FindUI(L"hacking_progressbar"));
    if (m_pProgressBar)
    {
        m_pProgressBar->SetVisible(true);
        m_pProgressBar->SetLoadingRatio(0.f);
    }

    m_pFastMarker = static_cast<UIImage*>(m_pEngineUtility->FindUI(L"hacking_fastmarker"));
    if (m_pFastMarker)
    {
        m_pFastMarker->SetVisible(true);

        // ★ fastmarker / progressbar 위치에서 시작·끝 X 계산
        UI_DESC barDesc = m_pProgressBar->GetUIDesc();
        UI_DESC markerDesc = m_pFastMarker->GetUIDesc();

        // bar 왼쪽을 start, 오른쪽을 end 로 사용 (marker 크기 고려해서 조금 안쪽으로 잡고 싶으면 +/- 조정)
        _float barLeft = barDesc.x - barDesc.w * 0.5f;
        _float barRight = barDesc.x + barDesc.w * 0.5f;

        m_fastStartX = barLeft;
        m_fastEndX = barRight;

        // 시작 위치로 강제 세팅
        markerDesc.x = m_fastStartX;
        m_pFastMarker->ApplyUIDesc(markerDesc);
    }


	m_pEngineUtility->FindUI(L"hacking_close_default")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_close_on")->SetVisible(false);
    if (auto pCloseBtn = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_close")))
    {
        pCloseBtn->SetEnable(true);
        pCloseBtn->ClearButtonFunctions();
        pCloseBtn->AddButtonFunction([this]()
            {
                this->CloseGame();
            });
        pCloseBtn->SetDefaultImage(L"hacking_close_default");
        pCloseBtn->SetOnImage(L"hacking_close_on");
    }


	m_pEngineUtility->FindUI(L"hacking_minimize_default")->SetVisible(true);
	m_pEngineUtility->FindUI(L"hacking_minimize_on")->SetVisible(false);
    if (auto pMinBtn = dynamic_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_minimize")))
    {
        pMinBtn->SetEnable(true);
        pMinBtn->ClearButtonFunctions();
        pMinBtn->AddButtonFunction([this]()
            {
                this->MinimizeGame();
            });
        pMinBtn->SetDefaultImage(L"hacking_minimize_default");
        pMinBtn->SetOnImage(L"hacking_minimize_on");
    }

	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_up_00"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_up_01"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_up_02"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_up_10"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_up_11"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_up_12"))->SetEnable(true);

	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_down_00"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_down_01"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_down_02"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_down_10"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_down_11"))->SetEnable(true);
	static_cast<UIButton*>(m_pEngineUtility->FindUI(L"hacking_down_12"))->SetEnable(true);

    InitGame();

    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(true);

    return S_OK;
}

void hackingGameUI::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_progressCur < m_progressTarget)
    {
        m_progressCur += m_progressSpeed * fTimeDelta;
        if (m_progressCur > m_progressTarget)
            m_progressCur = m_progressTarget;
    }

    if (m_pProgressBar)
    {
        m_pProgressBar->SetLoadingRatio(m_progressCur);
    }

    if (!m_finished && !m_allSolved && !m_minimized)
    {
        m_timeAcc += fTimeDelta;

        _float t = m_timeAcc / m_timeLimit;
        if (t > 1.f) t = 1.f;

        if (m_pFastMarker)
        {
            UI_DESC markerDesc = m_pFastMarker->GetUIDesc();
            markerDesc.x = m_fastStartX + (m_fastEndX - m_fastStartX) * t;
            m_pFastMarker->ApplyUIDesc(markerDesc);
        }

        if (m_timeAcc >= m_timeLimit)
        {
            CloseGame();
            return;
        }
    }

    if (m_allSolved && !m_finished)
    {
        if (m_progressCur >= 1.f - 0.0001f)
        {
            m_clearWaitAcc += fTimeDelta;
            if (m_clearWaitAcc >= m_clearWaitDuration)
            {
                OnGameClear();
                return;
            }
        }
    }

    if (m_finished)
        return;
}

void hackingGameUI::SetPanel(Object* pPanel)
{
    m_pPanel = pPanel;
}

void hackingGameUI::Unlock()
{
    if(m_pPanel)
        static_cast<Panel*>(m_pPanel)->UnlockDoor();
}

void hackingGameUI::ShowGame()
{
    SetVisibleAndEnable(L"hacking_back", true, false);
    SetVisibleAndEnable(L"hacking_gameplate", true, false);
    SetVisibleAndEnable(L"hacking_done1", true, false);
    SetVisibleAndEnable(L"hacking_done2", true, false);
    SetVisibleAndEnable(L"hacking_start1", true, false);
    SetVisibleAndEnable(L"hacking_start2", true, false);
    SetVisibleAndEnable(L"hacking_goal1", true, false);
    SetVisibleAndEnable(L"hacking_goal2", true, false);
    SetVisibleAndEnable(L"hacking_back_text", true, false);
    SetVisibleAndEnable(L"hacking_gameplate_text", true, false);
    SetVisibleAndEnable(L"hacking_progress", true, false);

    SetVisibleAndEnable(L"hacking_close_default", true, false);
    SetVisibleAndEnable(L"hacking_close_on", false, false);
    SetVisibleAndEnable(L"hacking_close", true, true);
    SetVisibleAndEnable(L"hacking_minimize_default", true, false);
    SetVisibleAndEnable(L"hacking_minimize_on", false, false);
    SetVisibleAndEnable(L"hacking_minimize", true, true);

    // 파이프 버튼 다시 활성화
    SetVisibleAndEnable(L"hacking_up_00", true, true);
    SetVisibleAndEnable(L"hacking_up_01", true, true);
    SetVisibleAndEnable(L"hacking_up_02", true, true);
    SetVisibleAndEnable(L"hacking_up_10", true, true);
    SetVisibleAndEnable(L"hacking_up_11", true, true);
    SetVisibleAndEnable(L"hacking_up_12", true, true);

    SetVisibleAndEnable(L"hacking_down_00", true, true);
    SetVisibleAndEnable(L"hacking_down_01", true, true);
    SetVisibleAndEnable(L"hacking_down_02", true, true);
    SetVisibleAndEnable(L"hacking_down_10", true, true);
    SetVisibleAndEnable(L"hacking_down_11", true, true);
    SetVisibleAndEnable(L"hacking_down_12", true, true);

    SetVisibleAndEnable(L"hacking_progressbar", true, true);
    SetVisibleAndEnable(L"hacking_fastmarker", true, true);

    // 파이프 이미지 다시 표시 (rotation, baseMask 등 기존 상태 그대로)
    for (_uint line = 0; line < LINE; ++line)
    {
        for (_uint r = 0; r < ROWS; ++r)
        {
            for (_uint c = 0; c < COLS; ++c)
            {
                if (m_cells[line][r][c].pImage)
                    m_cells[line][r][c].pImage->SetVisible(true);
            }
        }
    }

    m_minimized = false;
    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(true);
}

void hackingGameUI::MinimizeGame()
{
    SetVisibleAndEnable(L"hacking_back", false, false);
    SetVisibleAndEnable(L"hacking_gameplate", false, false);
    SetVisibleAndEnable(L"hacking_done1", false, false);
    SetVisibleAndEnable(L"hacking_done2", false, false);
    SetVisibleAndEnable(L"hacking_start1", false, false);
    SetVisibleAndEnable(L"hacking_start2", false, false);
    SetVisibleAndEnable(L"hacking_goal1", false, false);
    SetVisibleAndEnable(L"hacking_goal2", false, false);
    SetVisibleAndEnable(L"hacking_back_text", false, false);
    SetVisibleAndEnable(L"hacking_gameplate_text", false, false);
    SetVisibleAndEnable(L"hacking_progress", false, false);

    SetVisibleAndEnable(L"hacking_close_default", false, false);
    SetVisibleAndEnable(L"hacking_close_on", false, false);
    SetVisibleAndEnable(L"hacking_close", false, false);
    SetVisibleAndEnable(L"hacking_minimize_default", false, false);
    SetVisibleAndEnable(L"hacking_minimize_on", false, false);
    SetVisibleAndEnable(L"hacking_minimize", false, false);

    // 파이프 버튼 비활성화
    SetVisibleAndEnable(L"hacking_up_00", false, false);
    SetVisibleAndEnable(L"hacking_up_01", false, false);
    SetVisibleAndEnable(L"hacking_up_02", false, false);
    SetVisibleAndEnable(L"hacking_up_10", false, false);
    SetVisibleAndEnable(L"hacking_up_11", false, false);
    SetVisibleAndEnable(L"hacking_up_12", false, false);

    SetVisibleAndEnable(L"hacking_down_00", false, false);
    SetVisibleAndEnable(L"hacking_down_01", false, false);
    SetVisibleAndEnable(L"hacking_down_02", false, false);
    SetVisibleAndEnable(L"hacking_down_10", false, false);
    SetVisibleAndEnable(L"hacking_down_11", false, false);
    SetVisibleAndEnable(L"hacking_down_12", false, false);

    SetVisibleAndEnable(L"hacking_progressbar", false, false);
    SetVisibleAndEnable(L"hacking_fastmarker", false, false);

    // 파이프 이미지도 안 보이게만 (상태는 그대로 유지)
    for (_uint line = 0; line < LINE; ++line)
    {
        for (_uint r = 0; r < ROWS; ++r)
        {
            for (_uint c = 0; c < COLS; ++c)
            {
                if (m_cells[line][r][c].pImage)
                    m_cells[line][r][c].pImage->SetVisible(false);
            }
        }
    }

    m_minimized = true;
    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(false);
}

hackingGameUI* hackingGameUI::Create()
{
    hackingGameUI* pInstance = new hackingGameUI();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : hackingGameUI");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* hackingGameUI::Clone(void* pArg)
{
    hackingGameUI* pInstance = new hackingGameUI(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : hackingGameUI");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void hackingGameUI::Free()
{
	__super::Free();
}

void hackingGameUI::InitGame()
{
    // [line][row][col]
    static const wchar_t* PIPE_BTN_NAMES[LINE][ROWS][COLS] =
    {
        {   // line 0 : up
            { L"hacking_up_00",   L"hacking_up_01",   L"hacking_up_02" },
            { L"hacking_up_10",   L"hacking_up_11",   L"hacking_up_12" },
        },
        {   // line 1 : down
            { L"hacking_down_00", L"hacking_down_01", L"hacking_down_02" },
            { L"hacking_down_10", L"hacking_down_11", L"hacking_down_12" },
        }
    };

    struct Pos
    {
        int r, c;
    };

    for (_uint line = 0; line < LINE; ++line)
    {
        m_lineSolved[line] = false;

        // ----------------------------------------------------
        // 1) (0,0) -> (ROWS-1, COLS-1) 까지 랜덤 경로 만들기
        // ----------------------------------------------------
        unsigned char required[ROWS][COLS] = {};
        Pos start{ 0, 0 };
        Pos goal{ (int)ROWS - 1, (int)COLS - 1 };

        const int MAX_ATTEMPT = 64;
        bool pathOk = false;
        std::vector<Pos> path;
        path.reserve(ROWS * COLS);

        for (int attempt = 0; attempt < MAX_ATTEMPT && !pathOk; ++attempt)
        {
            bool visited[ROWS][COLS] = {};
            path.clear();

            visited[start.r][start.c] = true;
            path.push_back(start);

            while (true)
            {
                Pos cur = path.back();
                if (cur.r == goal.r && cur.c == goal.c)
                {
                    pathOk = true;
                    break;
                }

                Pos candidates[4];
                int candCount = 0;

                auto tryAdd = [&](int r, int c)
                    {
                        if (r < 0 || r >= (int)ROWS || c < 0 || c >= (int)COLS)
                            return;
                        if (visited[r][c])
                            return;
                        candidates[candCount++] = { r, c };
                    };

                // 이웃(상하좌우) 수집
                tryAdd(cur.r, cur.c + 1); // right
                tryAdd(cur.r, cur.c - 1); // left
                tryAdd(cur.r - 1, cur.c); // up
                tryAdd(cur.r + 1, cur.c); // down

                if (candCount == 0)
                    break; // 막혔으니 이 attempt 실패, 다음 attempt

                Pos next = candidates[rand() % candCount];
                visited[next.r][next.c] = true;
                path.push_back(next);
            }
        }

        // 만약 랜덤 경로 생성 실패 시, 고정된 안전 경로 사용
        if (!pathOk)
        {
            path.clear();
            path.push_back(start);              // (0,0)
            path.push_back(Pos{ 0, 1 });        // (0,1)
            path.push_back(Pos{ 0, 2 });        // (0,2)
            path.push_back(goal);               // (1,2)
        }

        // ----------------------------------------------------
        // 2) 경로를 바탕으로 requiredMask 계산
        // ----------------------------------------------------
        memset(required, 0, sizeof(required));

        const unsigned char UP = (1 << DIR_UP);
        const unsigned char RIGHT = (1 << DIR_RIGHT);
        const unsigned char DOWN = (1 << DIR_DOWN);
        const unsigned char LEFT = (1 << DIR_LEFT);

        // 시작/끝 외부 연결
        required[start.r][start.c] |= LEFT;   // 00의 왼쪽
        required[goal.r][goal.c] |= RIGHT;  // 12의 오른쪽

        // 경로상의 인접칸들끼리 연결 방향 추가
        for (size_t i = 0; i + 1 < path.size(); ++i)
        {
            Pos a = path[i];
            Pos b = path[i + 1];

            if (a.r == b.r)
            {
                if (b.c == a.c + 1)
                {
                    required[a.r][a.c] |= RIGHT;
                    required[b.r][b.c] |= LEFT;
                }
                else if (b.c == a.c - 1)
                {
                    required[a.r][a.c] |= LEFT;
                    required[b.r][b.c] |= RIGHT;
                }
            }
            else if (a.c == b.c)
            {
                if (b.r == a.r + 1)
                {
                    required[a.r][a.c] |= DOWN;
                    required[b.r][b.c] |= UP;
                }
                else if (b.r == a.r - 1)
                {
                    required[a.r][a.c] |= UP;
                    required[b.r][b.c] |= DOWN;
                }
            }
        }

        // ----------------------------------------------------
        // 3) requiredMask에 맞게 각 셀의 baseMask / rotation 설정
        //    + UI 버튼 / 이미지 생성
        // ----------------------------------------------------
        unsigned char baseLine = MakeMask(false, true, false, true); // L-R
        unsigned char baseCurve = MakeMask(false, false, true, true); // D-L

        for (_uint r = 0; r < ROWS; ++r)
        {
            for (_uint c = 0; c < COLS; ++c)
            {
                Cell& cell = m_cells[line][r][c];
                cell.baseMask = 0;
                cell.rotation = 0;
                cell.pButton = nullptr;
                cell.pImage = nullptr;
                cell.isCurve = false;

                // 버튼 찾기
                cell.pButton = dynamic_cast<UIButton*>(
                    m_pEngineUtility->FindUI(PIPE_BTN_NAMES[line][r][c]));
                if (!cell.pButton)
                    continue;

                // 버튼 클릭 콜백 등록
                cell.pButton->ClearButtonFunctions();
                cell.pButton->AddButtonFunction([this, line, r, c]()
                    {
                        this->OnCellClick(line, r, c);
                    });

                unsigned char req = required[r][c];

                int solvedRot = 0; // "정답" 회전값

                if (req != 0)
                {
                    // --- 메인 경로 위의 셀: reqMask에 딱 맞는 타일(직선/곡선+회전) 선택 ---
                    struct Candidate
                    {
                        bool isCurve;
                        unsigned char baseMask;
                        int rot;
                    };
                    std::vector<Candidate> candidates;

                    auto addCandidate = [&](bool isCurve, unsigned char baseMask)
                        {
                            for (int rot = 0; rot < 4; ++rot)
                            {
                                unsigned char m = GetRotatedMask(baseMask, rot);
                                if (m == req)
                                {
                                    candidates.push_back({ isCurve, baseMask, rot });
                                }
                            }
                        };

                    addCandidate(false, baseLine);
                    addCandidate(true, baseCurve);

                    Candidate chosen{};
                    if (!candidates.empty())
                    {
                        chosen = candidates[rand() % candidates.size()];
                    }
                    else
                    {
                        // 혹시라도 못찾으면 수평 직선 기본값
                        chosen.isCurve = false;
                        chosen.baseMask = baseLine;
                        chosen.rot = 0;
                    }

                    cell.isCurve = chosen.isCurve;
                    cell.baseMask = chosen.baseMask;
                    solvedRot = chosen.rot;
                }
                else
                {
                    // --- 메인 경로에 속하지 않는 셀: 완전 랜덤 ---
                    cell.isCurve = (rand() % 2) == 1;
                    cell.baseMask = cell.isCurve ? baseCurve : baseLine;
                    solvedRot = rand() % 4; // 이 셀은 어차피 요구조건 없음
                }

                // 시작 상태는 "정답 회전 + 랜덤 오프셋" 으로 섞어 둔다.
                int scramble = rand() % 4;
                cell.rotation = (solvedRot + scramble) & 3;

                // 버튼 위치/크기로 UIImage 생성
                auto btnDesc = cell.pButton->GetUIDesc();

                std::string imgName;
                if (line == 0)
                    imgName = "hacking_pipe_up_" + std::to_string(r * COLS + c);
                else
                    imgName = "hacking_pipe_down_" + std::to_string(r * COLS + c);

                btnDesc.name = imgName;
                btnDesc.imagePath = cell.isCurve
                    ? L"../bin/Resources/Textures/hacking/curve.png"
                    : L"../bin/Resources/Textures/hacking/line.png";
                btnDesc.visible = true;

                m_pEngineUtility->AddObject(
                    SCENE::STATIC,
                    TEXT("UIImage"),
                    SCENE::GAMEPLAY,
                    L"UI",
                    &btnDesc
                );

                UI* pNewUI =
                    static_cast<UI*>(m_pEngineUtility
                        ->FindLayer(SCENE::GAMEPLAY, L"UI")
                        ->GetAllObjects().back());

                auto pImg = dynamic_cast<Engine::UIImage*>(pNewUI);
                cell.pImage = pImg;

                if (pImg)
                {
                    m_pEngineUtility->AddUI(
                        std::wstring(imgName.begin(), imgName.end()).c_str(),
                        pImg
                    );

                    // 현재 rotation 값 적용 (섞인 상태)
                    pImg->SetRotationRad(cell.rotation * XM_PIDIV2);
                }
            }
        }

        // 시작 시에는 항상 미완성 상태로 두고, 불 꺼진 상태로 연출
        m_lineSolved[line] = false;
        UpdateLineVisual(line);
    }
}


void hackingGameUI::CloseGame()
{
    if (m_finished == false)
    {
        // 실패/포기 시 별도 연출 들어갈 자리
    }

    SetVisibleAndEnable(L"hacking_back", false, false);
    SetVisibleAndEnable(L"hacking_gameplate", false, false);
    SetVisibleAndEnable(L"hacking_done1", false, false);
    SetVisibleAndEnable(L"hacking_done2", false, false);
    SetVisibleAndEnable(L"hacking_start1", false, false);
    SetVisibleAndEnable(L"hacking_start2", false, false);
    SetVisibleAndEnable(L"hacking_goal1", false, false);
    SetVisibleAndEnable(L"hacking_goal2", false, false);
    SetVisibleAndEnable(L"hacking_back_text", false, false);
    SetVisibleAndEnable(L"hacking_gameplate_text", false, false);
    SetVisibleAndEnable(L"hacking_progress", false, false);

    SetVisibleAndEnable(L"hacking_close_default", false, false);
    SetVisibleAndEnable(L"hacking_close_on", false, false);
    SetVisibleAndEnable(L"hacking_close", false, false);
    SetVisibleAndEnable(L"hacking_minimize_default", false, false);
    SetVisibleAndEnable(L"hacking_minimize_on", false, false);
    SetVisibleAndEnable(L"hacking_minimize", false, false);

    // 파이프 버튼 비활성화
    SetVisibleAndEnable(L"hacking_up_00", false, false);
    SetVisibleAndEnable(L"hacking_up_01", false, false);
    SetVisibleAndEnable(L"hacking_up_02", false, false);
    SetVisibleAndEnable(L"hacking_up_10", false, false);
    SetVisibleAndEnable(L"hacking_up_11", false, false);
    SetVisibleAndEnable(L"hacking_up_12", false, false);

    SetVisibleAndEnable(L"hacking_down_00", false, false);
    SetVisibleAndEnable(L"hacking_down_01", false, false);
    SetVisibleAndEnable(L"hacking_down_02", false, false);
    SetVisibleAndEnable(L"hacking_down_10", false, false);
    SetVisibleAndEnable(L"hacking_down_11", false, false);
    SetVisibleAndEnable(L"hacking_down_12", false, false);

    SetVisibleAndEnable(L"hacking_progressbar", false, false);
    SetVisibleAndEnable(L"hacking_fastmarker", false, false);

    // 생성해 둔 파이프 이미지 제거
    for (_uint line = 0; line < LINE; ++line)
    {
        for (_uint r = 0; r < ROWS; ++r)
        {
            for (_uint c = 0; c < COLS; ++c)
            {
                if (m_cells[line][r][c].pImage)
                {
                    m_cells[line][r][c].pImage->SetVisible(false);
                    m_cells[line][r][c].pImage->SetDead(true);
                    m_cells[line][r][c].pImage = nullptr;
                }
            }
        }
    }

    m_pEngineUtility->RemoveUI(L"hackingGameUI");
    SetDead(true);
    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), L"Player", 0);
    static_cast<Player*>(pPlayer)->SetPlayingMinigame(false);
}

void hackingGameUI::OnGameClear()
{
    m_finished = true;
    Unlock();
    if (m_pPanel)
        static_cast<Panel*>(m_pPanel)->SetVisiblePanelUI(true);
    CloseGame();
}

void hackingGameUI::OnCellClick(_uint line, _uint row, _uint col)
{
    if (m_finished)
        return;
    if (line >= LINE || row >= ROWS || col >= COLS)
        return;

    if (m_lineSolved[line])
        return;

    Cell& cell = m_cells[line][row][col];

    // 90도 시계 방향 회전
    cell.rotation = (cell.rotation + 1) & 3;

    if (cell.pImage)
    {
        cell.pImage->SetRotationRad(cell.rotation * XM_PIDIV2);
    }

    // 해당 line(up/down)이 연결됐는지 체크
    bool prevSolved = m_lineSolved[line];
    bool nowSolved = CheckLine(line);
    m_lineSolved[line] = nowSolved;
    UpdateLineVisual(line);

    if (nowSolved && !prevSolved)
    {
        _float newTarget = 0.f;
        if (m_lineSolved[0]) newTarget += 0.5f;
        if (m_lineSolved[1]) newTarget += 0.5f;

        m_progressTarget = newTarget;
        if (m_progressTarget > 1.f)
            m_progressTarget = 1.f;
    }

    // 두 줄 다 완료되면 즉시 종료 X, progress 애니메이션 끝날 때까지 대기
    if (m_lineSolved[0] && m_lineSolved[1])
    {
        m_allSolved = true;
        m_clearWaitAcc = 0.f;
    }
}

unsigned char hackingGameUI::GetRotatedMask(unsigned char baseMask, _int rotation) const
{
    rotation &= 3;
    if (rotation == 0)
        return baseMask;

    unsigned char result = 0;
    for (int dir = 0; dir < 4; ++dir)
    {
        if (baseMask & (1 << dir))
        {
            int newDir = (dir - rotation + 4) & 3; // 시계 방향 회전
            result |= (1 << newDir);
        }
    }
    return result;
}

_bool hackingGameUI::CheckLine(_uint line) const
{
    if (line >= LINE)
        return false;

    const unsigned char UP = (1 << DIR_UP);
    const unsigned char RIGHT = (1 << DIR_RIGHT);
    const unsigned char DOWN = (1 << DIR_DOWN);
    const unsigned char LEFT = (1 << DIR_LEFT);

    auto maskAt = [&](int r, int c) -> unsigned char
        {
            return GetRotatedMask(
                m_cells[line][r][c].baseMask,
                m_cells[line][r][c].rotation
            );
        };

    // 시작/끝 셀 비트 조건
    unsigned char mStart = maskAt(0, 0);                  // 00
    unsigned char mGoal = maskAt(ROWS - 1, COLS - 1);    // 12

    if ((mStart & LEFT) == 0)
        return false;
    if ((mGoal & RIGHT) == 0)
        return false;

    // 2x3 BFS
    bool visited[ROWS][COLS] = {};
    std::queue<std::pair<int, int>> q;

    visited[0][0] = true;
    q.push({ 0, 0 });

    while (!q.empty())
    {
        auto [r, c] = q.front();
        q.pop();

        unsigned char mCur = maskAt(r, c);

        auto tryPush = [&](int nr, int nc, unsigned char needFromCur, unsigned char needFromNext)
            {
                if (nr < 0 || nr >= (int)ROWS || nc < 0 || nc >= (int)COLS)
                    return;
                if (visited[nr][nc])
                    return;

                unsigned char mNext = maskAt(nr, nc);
                if ((mCur & needFromCur) && (mNext & needFromNext))
                {
                    visited[nr][nc] = true;
                    q.push({ nr, nc });
                }
            };

        // 오른쪽
        tryPush(r, c + 1, RIGHT, LEFT);
        // 왼쪽
        tryPush(r, c - 1, LEFT, RIGHT);
        // 위쪽
        tryPush(r - 1, c, UP, DOWN);
        // 아래쪽
        tryPush(r + 1, c, DOWN, UP);
    }

    return visited[ROWS - 1][COLS - 1];   // 12까지 도달했는지
}

void hackingGameUI::UpdateLineVisual(_uint line)
{
    if (line >= LINE)
        return;

    const wchar_t* doneName = (line == 0) ? L"hacking_done1" : L"hacking_done2";

    UI* pUI = m_pEngineUtility->FindUI(doneName);
    if (auto pImg = dynamic_cast<UIImage*>(pUI))
    {
        if (m_lineSolved[line])
        {
            pImg->SetBrightness(1.f);
            pImg->SetGamma(1.f);
        }
        else
        {
            pImg->SetBrightness(0.1f);
            pImg->SetGamma(1.f);
        }
    }
}

void hackingGameUI::SetVisibleAndEnable(const wchar_t* name, _bool visible, _bool enableButton)
{
    UI* pUI = m_pEngineUtility->FindUI(name);
    if (!pUI)
        return;

    pUI->SetVisible(visible);

    if (auto pBtn = dynamic_cast<UIButton*>(pUI))
        pBtn->SetEnable(enableButton);
}
