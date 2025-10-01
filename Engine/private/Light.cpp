#include "Light.h"

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
