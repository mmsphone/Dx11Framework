#pragma once

#include "base.h"

NS_BEGIN(Engine)

class ShadowLightManager final : public Base
{
	ShadowLightManager();
	virtual ~ShadowLightManager() = default;

public:
	HRESULT AddShadowLight(const SHADOW_DESC& ShadowDesc);
	const _float4x4* GetShadowTransformFloat4x4Ptr(D3DTS eState, _uint iIndex);
	const _float3* GetShadowLightPositionPtr(_uint iIndex);
	const _float* GetShadowLightFarDistancePtr(_uint iIndex);
	void ClearShadowLights();
	_uint GetNumShadowLights();

	static ShadowLightManager* Create();
	virtual void Free() override;
private:
	list<_float4x4*>				m_pTransformationMatrices[D3DTS_END] = {};
	list<_float3*>					m_vLightPosition{};
	list<_float*>					m_fFarDistance{};
};

NS_END