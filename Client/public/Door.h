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
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Door* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents();

	_float scaleOffset = 0.015f;
};

NS_END