#pragma once

#include "base.h"

NS_BEGIN(Engine)

class ShadowLightManager final : public Base
{
	ShadowLightManager();
	virtual ~ShadowLightManager() = default;

public:
	const SHADOW_DESC* GetShadowLight(_uint iIndex);
	HRESULT AddShadowLight(const SHADOW_DESC& ShadowDesc);
	HRESULT RemoveShadowLight(_uint iIndex);
	const list<class ShadowLight*>& GetAllShadowLights();
	const list<class ShadowLight*>& GetActiveShadowLights();
	void ClearShadowLights();

	const _float4x4* GetActiveShadowLightTransformFloat4x4Ptr(D3DTS eState, _uint iIndex);
	const _float3* GetActiveShadowLightPositionPtr(_uint iIndex);
	const _float* GetActiveShadowLightFarDistancePtr(_uint iIndex);
	
	_uint GetNumActiveShadowLights();

	void SetShadowLightActive(_uint iIndex, _bool bActive);
	void SetShadowLightActive(class ShadowLight* pShadowLight, _bool bActive);
	void SetActiveShadowLightsByDistance(_fvector vPos, _float fMaxDistance, _uint iMaxCount);


	static ShadowLightManager* Create();
	virtual void Free() override;
private:
	list<class ShadowLight*> m_ShadowLights;
	list<class ShadowLight*> m_ActiveShadowLights;
};

NS_END