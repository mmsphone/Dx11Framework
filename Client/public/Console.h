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

	static Console* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents();

	_float scaleOffset = 0.015f;

	_bool  m_playerInRange = false;
	_bool  m_isActive = false;
	_uint m_waveIndex = 0;
	_float m_waveTimer = 0.f;
	vector<_float> m_waveDurations;
};

NS_END