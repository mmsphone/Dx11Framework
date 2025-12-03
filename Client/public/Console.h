#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Console final : public ObjectTemplate
{
	Console();
	Console(const Console& Prototype);
	virtual ~Console() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void StopWave();

	void UnlockConsole();
	void    SetVisibleConsoleUI(_bool bVisible);

	static Console* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents();

	HRESULT ReadyWaveUI();
	void UpdateWaveUI(_float fTimeDelta);
	_float GetWaveFillRatio() const;
	_int   GetRemainWaveCount() const;

	void    StartWaveSequence();

	_float scaleOffset = 0.015f;

	_bool  m_playerInRange = false;
	_bool  m_isActive = false;
	_uint m_waveIndex = 0;
	_float m_waveTimer = 0.f;
	_float m_currentWaveDuration = 0.f;
	vector<_float> m_waveDurations;

	_bool   m_waveUIReady = false;

	_bool  m_openedHackingUI = false;
	_float m_keyBlinkAcc = 0.f;
	_bool  m_keyBlinkOnState = false;

	_bool m_isLocked = true;
};

NS_END