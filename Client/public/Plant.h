#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Plant final : public ObjectTemplate
{
private:
	Plant();
	Plant(const Plant& Prototype);
	virtual ~Plant() = default;

public:
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Plant* Create();
	virtual Object* Clone(void* pArg) override;

private:
	HRESULT ReadyComponents();
};

NS_END