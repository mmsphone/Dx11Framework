#include "Light.h"

#include "EngineUtility.h"

Light::Light()
{
}

HRESULT Light::Initialize(const LIGHT_DESC& lightDesc)
{
    m_LightDesc = lightDesc;
    return S_OK;
}

const LIGHT_DESC* Light::GetLight() const
{
    return &m_LightDesc;
}

HRESULT Light::RenderLight(class Shader* pShader, class VIBufferRect* pVIBuffer)
{
	_uint		iPassIndex = {};

	if (LIGHT_DIRECTIONAL == m_LightDesc.eType)
	{
		if (FAILED(pShader->BindRawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 1;
	}
	else if( LIGHT_POINT == m_LightDesc.eType)
	{
		if (FAILED(pShader->BindRawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->BindRawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = 2;
	}

	if (FAILED(pShader->BindRawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->BindRawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->BindRawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);
	pVIBuffer->BindBuffers();
	pVIBuffer->Render();


	return S_OK;
}

Light* Light::Create(const LIGHT_DESC& lightDesc)
{
    Light* pInstance = new Light();

    if (FAILED(pInstance->Initialize(lightDesc)))
    {
        MSG_BOX("Failed to Created : Light");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Light::Free()
{
    __super::Free();
}
