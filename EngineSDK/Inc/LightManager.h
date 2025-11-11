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
	HRESULT RemoveLight(_uint iIndex);
	void ClearLights();
	HRESULT RenderLights(class Shader* pShader, class VIBufferRect* pVIBuffer);

	static LightManager* Create();
	virtual void Free() override;
private:
	list<class Light*> m_Lights;
};

NS_END