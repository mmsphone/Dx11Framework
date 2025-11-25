#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Door final : public ObjectTemplate
{
	Door();
	Door(const Door& Prototype);
	virtual ~Door() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void SetLock(_bool bLock);
	_bool IsLocked();
	void Open();

	static Door* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents();

	_float scaleOffset = 0.015f;

	_float3 m_closedPos{};      // 처음 위치
	_float3 m_openPos{};        // 열렸을 때 위치
	_bool   m_isOpening = false;
	_bool   m_isOpen = false;
	_float  m_openT = 0.f;
	_float  m_openDuration = 2.f;

	_bool m_posInitialized = false;

	_bool m_isLock = false;
};

NS_END