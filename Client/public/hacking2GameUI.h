#pragma once

#include "Client_Defines.h"
#include "UI.h"

NS_BEGIN(Engine)
class UIButton;
class UIImage;
class UILabel;
NS_END

NS_BEGIN(Client)

class Console;

class hacking2GameUI final : public UI
{
private:
    hacking2GameUI();
    hacking2GameUI(const hacking2GameUI& Prototype);
    virtual ~hacking2GameUI() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;

    void SetConsole(Console* pConsole);

    // 콘솔에서 호출
    void ShowGame();      // 처음 / Close 후 → 새 게임, Minimize 후 → 그대로 복귀
    void MinimizeGame();  // 진행 그대로, 창만 숨김
    void CloseGame();     // 완전 종료, 다시 열면 새 게임

    static hacking2GameUI* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void   Free() override;

private:
    // 초기 셋업
    HRESULT SetupButtons();
    HRESULT SetupNumberLabels();
    void    InitGame();

    // 매 프레임 로직
    void UpdateLines(_float fTimeDelta);      // 4줄 숫자 움직임
    void UpdateDoneEffect(_float fTimeDelta); // 클리어 후 done 연출

    // 버튼 콜백
    void OnClickDir(_uint index, _int dir); // dir: -1 위, +1 아래

    // 상태 체크
    _bool IsAllLinesAligned() const; // 4줄 모두 TARGET_ROW에 1이 있는지

    // UI 묶음 제어
    void SetWindowVisible(_bool visible); // 게임창 전체
    void SetDoneUIVisible(_bool visible); // done_* 관련만

    void OnGameClear();

    void UpdateLineButtonVisual(_uint index);
    void UpdateAllLineButtonVisual();

    void UpdateLineNumbers(_uint lineIndex);
    void UpdateAllLineNumbers();

private:
    Console* m_pConsole = nullptr;

    // 숫자 줄 정보
    static const _uint LINE_COUNT = 4;
    static const _uint ROW_COUNT = 9; // 한 줄 높이(필요하면 조정)

    struct LineState
    {
        _int   currentRow = 0; // 1이 있는 위치
        _int   dir = -1; // -1=위로, +1=아래로
        _float acc = 0.f; // 한 칸 이동용 누적시간
    };
    LineState m_lines[LINE_COUNT]{};

    struct LineControl
    {
        UIButton* pUp = nullptr; // hacking2_updownX_up
        UIButton* pDown = nullptr; // hacking2_updownX_down
    };
    LineControl m_controls[LINE_COUNT]{};

    _float m_scrollInterval = 1.f; // 한 칸 움직이는 간격(초)

    // 상태 플래그
    _bool m_active = false; // 숫자판 돌아가는 중
    _bool m_minimized = false; // 최소화 상태
    _bool m_finished = false; // 클리어 후 done 연출 중

    // done 프로그레스바
    _float           m_doneProgress = 0.f;    // 0~1
    _float           m_doneFillSpeed = 0.1f;
    UIImage* m_pDoneProgressBar = nullptr;

    UILabel* m_numberLabels[LINE_COUNT][ROW_COUNT]{};
};

NS_END
