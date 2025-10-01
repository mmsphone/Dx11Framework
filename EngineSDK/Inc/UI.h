#pragma once

#include "Object.h"

class ENGINE_DLL UI abstract : public Object
{
public:
	typedef struct tagUIDesc : public Object::OBJECT_DESC
	{
		_float fX;
		_float fY;
		_float fSizeX;
		_float fSizeY;
	}UI_DESC;
protected:
	UI();
	UI(const UI& Prototype);
	virtual ~UI() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	void UpdateState();

protected:
	_float						m_fX, m_fY, m_fSizeX, m_fSizeY;
	_float4x4					m_ViewMatrix{}, m_ProjMatrix{};

	_float						m_fViewportSizeX{}, m_fViewportSizeY{};
};

