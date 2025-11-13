#include "ShadowLightManager.h"

#include "ShadowLight.h"

ShadowLightManager::ShadowLightManager()
{
}

const SHADOW_DESC* ShadowLightManager::GetShadowLight(_uint iIndex)
{
    if (iIndex >= m_ShadowLights.size())
        return nullptr;

    auto it = m_ShadowLights.begin();
    std::advance(it, iIndex);
    ShadowLight* pShadowLight = *it;
    if (!pShadowLight)
        return nullptr;

    return pShadowLight->GetShadowLight();
}

HRESULT ShadowLightManager::AddShadowLight(const SHADOW_DESC& ShadowDesc)
{
    ShadowLight* pShadowLight = ShadowLight::Create(ShadowDesc);
    if (!pShadowLight)
        return E_FAIL;

    m_ShadowLights.push_back(pShadowLight);
    m_ActiveShadowLights.push_back(pShadowLight);

    return S_OK;
}

HRESULT ShadowLightManager::RemoveShadowLight(_uint iIndex)
{
    if (iIndex >= m_ShadowLights.size())
        return E_FAIL;

    auto it = m_ShadowLights.begin();
    std::advance(it, iIndex);
    ShadowLight* pTarget = *it;

    for (auto itAct = m_ActiveShadowLights.begin(); itAct != m_ActiveShadowLights.end(); )
    {
        if (*itAct == pTarget)
            itAct = m_ActiveShadowLights.erase(itAct);
        else
            ++itAct;
    }
    SafeRelease(pTarget);
    m_ShadowLights.erase(it);

    return S_OK;
}

const _float4x4* ShadowLightManager::GetActiveShadowLightTransformFloat4x4Ptr(D3DTS eState, _uint iIndex)
{
    if (iIndex >= m_ActiveShadowLights.size())
        return nullptr;

    auto it = m_ActiveShadowLights.begin();
    std::advance(it, iIndex);
    ShadowLight* pShadowLight = *it;
    if (!pShadowLight) 
        return nullptr;

    return pShadowLight->GetShadowLightTransformFloat4x4Ptr(eState);
}

const _float3* ShadowLightManager::GetActiveShadowLightPositionPtr(_uint iIndex)
{
    if (iIndex >= m_ActiveShadowLights.size())
        return nullptr;

    auto it = m_ActiveShadowLights.begin();
    std::advance(it, iIndex);
    ShadowLight* pShadowLight = *it;
    if (!pShadowLight)
        return nullptr;

    return pShadowLight->GetShadowLightPositionPtr();
}

const _float* ShadowLightManager::GetActiveShadowLightFarDistancePtr(_uint iIndex)
{
    if (iIndex >= m_ActiveShadowLights.size())
        return nullptr;

    auto it = m_ActiveShadowLights.begin();
    std::advance(it, iIndex);
    ShadowLight* pShadowLight = *it;
    if (!pShadowLight)
        return nullptr;

    return pShadowLight->GetShadowLightFarDistancePtr();
}

const list<class ShadowLight*>& ShadowLightManager::GetAllShadowLights()
{
    return m_ShadowLights;
}

const list<class ShadowLight*>& ShadowLightManager::GetActiveShadowLights()
{
    return m_ActiveShadowLights;
}

void ShadowLightManager::ClearShadowLights()
{
    for (auto& pShadowLight : m_ShadowLights)
        SafeRelease(pShadowLight);

    m_ShadowLights.clear();
    m_ActiveShadowLights.clear();
}

_uint ShadowLightManager::GetNumActiveShadowLights()
{
    _uint num = static_cast<_uint>(m_ActiveShadowLights.size());
    return num > 4 ? 4 : num;
}

void ShadowLightManager::SetShadowLightActive(_uint iIndex, _bool bActive)
{
    if (iIndex >= m_ShadowLights.size())
        return;

    auto it = m_ShadowLights.begin();
    std::advance(it, iIndex);
    ShadowLight* pShadowLight = *it;
    if (!pShadowLight) return;

    if (bActive)
    {
        // 이미 있으면 중복 추가 X
        for (auto& pActiveShadowLight : m_ActiveShadowLights)
        {
            if (pActiveShadowLight == pShadowLight)
                return;
        }
        m_ActiveShadowLights.push_back(pShadowLight);
    }
    else
    {
        for (auto itAct = m_ActiveShadowLights.begin(); itAct != m_ActiveShadowLights.end(); )
        {
            if (*itAct == pShadowLight)
                itAct = m_ActiveShadowLights.erase(itAct);
            else
                ++itAct;
        }
    }
}

void ShadowLightManager::SetShadowLightActive(ShadowLight* pShadowLight, _bool bActive)
{
    if (!pShadowLight)
        return;

    // 이 매니저가 소유한 라이트인지 확인 (옵션이지만 방어용)
    bool owned = false;
    for (auto& p : m_ShadowLights)
    {
        if (p == pShadowLight)
        {
            owned = true;
            break;
        }
    }
    if (!owned)
        return;

    if (bActive)
    {
        // 이미 활성 리스트에 있으면 중복 추가 X
        for (auto& pAct : m_ActiveShadowLights)
        {
            if (pAct == pShadowLight)
                return;
        }
        m_ActiveShadowLights.push_back(pShadowLight);
    }
    else
    {
        // 비활성: 활성 리스트에서 제거
        for (auto it = m_ActiveShadowLights.begin(); it != m_ActiveShadowLights.end(); )
        {
            if (*it == pShadowLight)
                it = m_ActiveShadowLights.erase(it);
            else
                ++it;
        }
    }
}

void ShadowLightManager::SetActiveShadowLightsByDistance(_fvector vPos, _float fMaxDistance, _uint iMaxCount)
{
    m_ActiveShadowLights.clear();

    const float maxDistSq = fMaxDistance * fMaxDistance;

    std::vector<std::pair<float, ShadowLight*>> candidates;
    candidates.reserve(m_ShadowLights.size());

    for (auto& pShadowLight : m_ShadowLights)
    {
        if (!pShadowLight)
            continue;

        const SHADOW_DESC* pDesc = pShadowLight->GetShadowLight();
        if (!pDesc)
            continue;

        _vector vEye = XMLoadFloat3(&pDesc->vEye);
        _vector diff = XMVectorSubtract(vEye, vPos);
        float distSq = XMVectorGetX(XMVector3LengthSq(diff));

        if (distSq <= maxDistSq)
            candidates.emplace_back(distSq, pShadowLight);
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const std::pair<float, ShadowLight*>& a, const std::pair<float, ShadowLight*>& b)
        {
            return a.first < b.first;
        });

    _uint added = 0;
    for (auto& c : candidates)
    {
        m_ActiveShadowLights.push_back(c.second);
        ++added;

        if (iMaxCount > 0 && added >= iMaxCount)
            break;
    }
}

ShadowLightManager* ShadowLightManager::Create()
{
	return new ShadowLightManager();
}

void ShadowLightManager::Free()
{
	__super::Free();

    ClearShadowLights();
}
