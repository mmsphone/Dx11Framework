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
	const list<class Light*>& GetAllLights();
	const list<class Light*>& GetActiveLights();
	void ClearLights();
	
	HRESULT RenderLights(class Shader* pShader, class VIBufferRect* pVIBuffer);

	void SetLightActive(_uint iIndex, _bool bActive);
	void SetLightActive(class Light* pLight, _bool bActive);
	void SetActiveLightsByDistance(_fvector vPos, _float fMaxDistance, _uint iMaxLights);

	static LightManager* Create();
	virtual void Free() override;
private:
	list<class Light*> m_Lights;
	list<class Light*> m_ActiveLights;
};

NS_END