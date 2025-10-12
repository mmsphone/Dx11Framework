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
HRESULT LightManager::RemoveLight(_uint iIndex)
{
    if (iIndex >= m_Lights.size())
        return E_FAIL;

    auto iter = m_Lights.begin();
    std::advance(iter, iIndex);

    SafeRelease(*iter);
    m_Lights.erase(iter);

    return S_OK;
}
void LightManager::ClearLights()
{
    for (auto& pLight : m_Lights)
        SafeRelease(pLight);

    m_Lights.clear();
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
