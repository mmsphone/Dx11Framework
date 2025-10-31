#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class CageCamera final : public ObjectTemplate
{
private:
	CageCamera();
	CageCamera(const CageCamera& Prototype);
	virtual ~CageCamera() = default;

public:
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static CageCamera* Create();
	virtual Object* Clone(void* pArg) override;

private:
	HRESULT ReadyComponents();
};

NS_END