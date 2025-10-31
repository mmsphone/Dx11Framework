#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class WoodenFence final : public ObjectTemplate
{
private:
	WoodenFence();
	WoodenFence(const WoodenFence& Prototype);
	virtual ~WoodenFence() = default;

public:
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static WoodenFence* Create();
	virtual Object* Clone(void* pArg) override;

private:
	HRESULT ReadyComponents();
};

NS_END