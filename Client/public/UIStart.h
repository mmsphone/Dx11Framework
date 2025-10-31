#pragma once

#include "UITemplate.h"

NS_BEGIN(Client)

class UIStart final : public UITemplate
{
	UIStart();
	UIStart(const UIStart& Prototype);
	virtual ~UIStart() = default;

public:
	virtual HRESULT Render() override;

	static UIStart* Create();
	virtual Object* Clone(void* pArg) override;

private:
	HRESULT ReadyComponents();
};

NS_END