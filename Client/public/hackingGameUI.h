#pragma once

#include "Client_Defines.h"
#include "UI.h"

NS_BEGIN(Engine)
class UIButton;
class UIImage;
NS_END

NS_BEGIN(Client)

class hackingGameUI : public UI
{
	hackingGameUI();
	hackingGameUI(const hackingGameUI& Prototype);
	virtual ~hackingGameUI() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;

	void SetPanel(Object* pPanel);
	void Unlock();

	void ShowGame();
	void MinimizeGame();

	static hackingGameUI* Create();
	Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	void InitGame();
	void CloseGame();
	void OnGameClear();

	void OnCellClick(_uint line, _uint row, _uint col, _int dirStep);
	unsigned char GetRotatedMask(unsigned char baseMask, _int rotation) const;
	_bool         CheckLine(_uint line) const;
	void          UpdateLineVisual(_uint line);

	void SetVisibleAndEnable(const wchar_t* name, _bool visible, _bool enableButton);

private:
	//게임 시작한 패널 저장
	Object* m_pPanel = { nullptr };

	//게임용
	static const _uint LINE = 2;
	static const _uint ROWS = 2;
	static const _uint COLS = 3;
	struct Cell
	{
		unsigned char     baseMask = 0;   // bit0=Up, bit1=Right, bit2=Down, bit3=Left
		_int              rotation = 0;
		UIButton* pButton = nullptr;
		UIImage* pImage = nullptr;
		_bool isCurve = false;
	};
	Cell  m_cells[LINE][ROWS][COLS]{};
	_bool m_lineSolved[LINE] = { false, false };
	_bool m_finished = false;

	//진행도 바
	UIImage* m_pProgressBar = nullptr;
	const _float	m_progressSpeed = 1.f;
	_float           m_progressCur = 0.f;     // 실제 표시값(0~1)
	_float           m_progressTarget = 0.f;     // 목표값(0,0.5,1)
	_bool            m_allSolved = false;   // 두 줄 다 완료했는지
	const _float	m_clearWaitDuration = 0.5f;
	_float           m_clearWaitAcc = 0.f;     // 대기 누적 시간

	//시간 제한
	UIImage* m_pFastMarker = nullptr;
	_float m_timeLimit = 10.f;   // 제한 시간(초) – 원하면 값 바꿔
	_float m_timeAcc = 0.f;    // 누적 시간
	_float m_fastStartX = 0.f;    // fastmarker 시작 X
	_float m_fastEndX = 0.f;    // fastmarker 끝 X

	_bool    m_minimized = false;
};

NS_END