#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL ShadowLight final : public Base
{
private:
	ShadowLight();
	virtual ~ShadowLight() = default;

public:
	HRESULT Initialize(const SHADOW_DESC& shadowLightDesc);
	const SHADOW_DESC* GetShadowLight() const;

	const _float4x4* GetShadowLightTransformFloat4x4Ptr(D3DTS eState) const;
	const _float3* GetShadowLightPositionPtr() const;
	const _float* GetShadowLightFarDistancePtr() const;
	void UpdateFromDesc(const SHADOW_DESC& shadowLightDesc);

	static ShadowLight* Create(const SHADOW_DESC& shadowLightDesc);
	virtual void Free() override;

private:
	void SetShadowLightViewProjection();

private:
	SHADOW_DESC m_ShadowLightDesc = {};
	_float4x4   m_View{};
	_float4x4   m_Projection{};
};

NS_END