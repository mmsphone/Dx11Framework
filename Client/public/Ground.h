#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Ground final : public ObjectTemplate
{
private:
	Ground();
	Ground(const Ground& Prototype);
	virtual ~Ground() = default;

public:
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Ground* Create();
	virtual Object* Clone(void* pArg) override;

private:
	HRESULT ReadyComponents();
};

NS_END