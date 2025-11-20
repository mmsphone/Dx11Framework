#pragma once

#include "Object.h"

NS_BEGIN(Engine)

class ENGINE_DLL UI abstract : public Object
{
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

	void ApplyUIDesc(const UI_DESC& desc);
	const UI_DESC& GetUIDesc() const;
	void SetVisible(_bool bVisible);

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	void UpdateState();

protected:
	UI_DESC m_desc{};
	_float m_fViewportSizeX{}, m_fViewportSizeY{};
	_float4x4 m_ViewMatrix{}, m_ProjMatrix{};
};

NS_END