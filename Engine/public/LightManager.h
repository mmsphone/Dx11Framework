#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class LightManager final : public Base
{
private:
	LightManager();
	virtual ~LightManager() = default;

public:
	const LIGHT_DESC* GetLight(_uint iIndex);
	HRESULT AddLight(const LIGHT_DESC& lightDesc);

	static LightManager* Create();
	virtual void Free() override;
private:
	list<class Light*> m_Lights;
};

NS_END