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
const list<class Light*>& LightManager::GetAllLights()
{
    return m_Lights;
}
const list<class Light*>& LightManager::GetActiveLights()
{
    return m_ActiveLights;
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

HRESULT LightManager::RenderLights(class Shader* pShader, class VIBufferRect* pVIBuffer)
{
    for (auto& pLight : m_ActiveLights)
    {
        pLight->RenderLight(pShader, pVIBuffer);
    }
    return S_OK;
}

void LightManager::SetLightActive(_uint iIndex, _bool bActive)
{
    if (iIndex >= m_Lights.size())
        return;

    auto iter = m_Lights.begin();
    std::advance(iter, iIndex);
    Light* pLight = *iter;

    SetLightActive(pLight, bActive);
}

void LightManager::SetLightActive(Light* pLight, _bool bActive)
{
    if (!pLight)
        return;

    if (bActive)
    {
        for (auto& pActiveLight : m_ActiveLights)
        {
            if (pActiveLight == pLight)
                return;
        }
        m_ActiveLights.push_back(pLight);
    }
    else
    {
        for (auto it = m_ActiveLights.begin(); it != m_ActiveLights.end(); )
        {
            if (*it == pLight)
                it = m_ActiveLights.erase(it);
            else
                ++it;
        }
    }
}

void LightManager::SetActiveLightsByDistance(_fvector vPos, _float fMaxDistance, _uint iMaxLights)
{
    m_ActiveLights.clear();

    const float maxDistSq = fMaxDistance * fMaxDistance;

    // (distance^2, Light*)
    std::vector<std::pair<float, Light*>> candidates;
    candidates.reserve(m_Lights.size());

    for (auto& pLight : m_Lights)
    {
        if (!pLight)
            continue;

        const LIGHT_DESC* pDesc = pLight->GetLight();
        if (!pDesc)
            continue;

        if (pDesc->eType == LIGHT_DIRECTIONAL)
        {
            // 방향성 라이트는 항상 포함, 거리 0으로 취급해서 우선순위 맨 앞으로
            candidates.emplace_back(0.0f, pLight);
        }
        else
        {
            // 포인트/스폿은 거리 체크
            _float3 lightPos = { pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z };
            _vector vLightPos = XMLoadFloat3(&lightPos);

            _vector diff = XMVectorSubtract(vLightPos, vPos);
            float distSq = XMVectorGetX(XMVector3LengthSq(diff));

            if (distSq <= maxDistSq)
                candidates.emplace_back(distSq, pLight);
        }
    }

    // 거리 기준 정렬 (directional은 0으로 넣었으니 항상 앞에 옴)
    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<float, Light*>& a, const std::pair<float, Light*>& b)
        {
            return a.first < b.first;
        });

    _uint count = 0;
    for (auto& c : candidates)
    {
        m_ActiveLights.push_back(c.second);
        ++count;

        if (iMaxLights > 0 && count >= iMaxLights)
            break;
    }
}

void LightManager::Free()
{
    __super::Free();

    for (auto& pLight : m_Lights)
        SafeRelease(pLight);

    m_Lights.clear();
    m_ActiveLights.clear();
}
