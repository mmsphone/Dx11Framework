#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class TreeHead final : public ObjectTemplate
{
private:
	TreeHead();
	TreeHead(const TreeHead& Prototype);
	virtual ~TreeHead() = default;

public:
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static TreeHead* Create();
	virtual Object* Clone(void* pArg) override;

private:
	HRESULT ReadyComponents();
};

NS_END