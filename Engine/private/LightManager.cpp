#include "LightManager.h"
#include "Light.h"

LightManager::LightManager()
{
}

const LIGHT_DESC* LightManager::GetLight(_uint iIndex)
{
    if (iIndex >= m_Lights.size())
        return nullptr;

    auto iter = m_Lights.begin();

    for (_uint i = 0; i < iIndex; i++)
        ++iter;

    return (*iter)->GetLight();
}

HRESULT LightManager::AddLight(const LIGHT_DESC& lightDesc)
{
    Light* pLight = Light::Create(lightDesc);

    if (pLight == nullptr)
        return E_FAIL;

    m_Lights.push_back(pLight);

    return S_OK;
}
LightManager* LightManager::Create()
{
    return new LightManager();
}
void LightManager::Free()
{
    __super::Free();

    for (auto& pLight : m_Lights)
        SafeRelease(pLight);

    m_Lights.clear();
}
