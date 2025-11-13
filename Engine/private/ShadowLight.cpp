#include "ShadowLight.h"

ShadowLight::ShadowLight()
{
}

HRESULT ShadowLight::Initialize(const SHADOW_DESC& shadowLightDesc)
{
	m_ShadowLightDesc = shadowLightDesc;
	SetShadowLightViewProjection();

	return S_OK;
}

const SHADOW_DESC* ShadowLight::GetShadowLight() const
{
	return &m_ShadowLightDesc;
}

const _float4x4* ShadowLight::GetShadowLightTransformFloat4x4Ptr(D3DTS eState) const
{
	switch (eState)
	{
	case D3DTS_VIEW:       
		return &m_View;
	case D3DTS_PROJECTION: 
		return &m_Projection;
	default:
		return nullptr;
	}
}

const _float3* ShadowLight::GetShadowLightPositionPtr() const
{
	return &m_ShadowLightDesc.vEye;
}

const _float* ShadowLight::GetShadowLightFarDistancePtr() const
{
	return &m_ShadowLightDesc.fFar;
}

void ShadowLight::UpdateFromDesc(const SHADOW_DESC& shadowLightDesc)
{
	m_ShadowLightDesc = shadowLightDesc;
	SetShadowLightViewProjection();
}

ShadowLight* ShadowLight::Create(const SHADOW_DESC& shadowLightDesc)
{
	ShadowLight* pInstance = new ShadowLight();

	if (FAILED(pInstance->Initialize(shadowLightDesc)))
	{
		MSG_BOX("Failed to Created : ShadowLight");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void ShadowLight::Free()
{
	__super::Free();
}

void ShadowLight::SetShadowLightViewProjection()
{
	_float3 vUpDir = { 0.f, 1.f, 0.f };

	_matrix viewMatrix = XMMatrixLookAtLH(
		XMVectorSetW(XMLoadFloat3(&m_ShadowLightDesc.vEye), 1.f),
		XMVectorSetW(XMLoadFloat3(&m_ShadowLightDesc.vAt), 1.f),
		XMLoadFloat3(&vUpDir));
	XMStoreFloat4x4(&m_View, viewMatrix);

	_matrix projMatrix = XMMatrixPerspectiveFovLH(
		m_ShadowLightDesc.fFovy, m_ShadowLightDesc.fAspect,
		m_ShadowLightDesc.fNear, m_ShadowLightDesc.fFar);
	XMStoreFloat4x4(&m_Projection, projMatrix);
}
