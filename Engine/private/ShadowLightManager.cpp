#include "ShadowLightManager.h"

ShadowLightManager::ShadowLightManager()
{
}

HRESULT ShadowLightManager::AddShadowLight(const SHADOW_DESC& ShadowDesc)
{
    _float3     vUpDir = { 0.f, 1.f, 0.f };

    //LightPos
    _float3* lightPos = new _float3{};
    *lightPos = ShadowDesc.vEye;
    m_vLightPosition.push_back(lightPos);

    //FarDistance
    _float* farDistance = new _float{};
    *farDistance = ShadowDesc.fFar;
    m_fFarDistance.push_back(farDistance);

    //View
    _matrix viewMatrix = XMMatrixLookAtLH(
        XMVectorSetW(XMLoadFloat3(&ShadowDesc.vEye), 1.f),
        XMVectorSetW(XMLoadFloat3(&ShadowDesc.vAt), 1.f),
        XMLoadFloat3(&vUpDir));
    _float4x4* view = new _float4x4{};
    XMStoreFloat4x4(view, viewMatrix);
    m_pTransformationMatrices[D3DTS_VIEW].push_back(view);

    //Projection
    _matrix projectionMatrix = XMMatrixPerspectiveFovLH(ShadowDesc.fFovy, ShadowDesc.fAspect, ShadowDesc.fNear, ShadowDesc.fFar);
    _float4x4* projection = new _float4x4{};
    XMStoreFloat4x4(projection, projectionMatrix);
    m_pTransformationMatrices[D3DTS_PROJECTION].push_back(projection);

    return S_OK;
}

const _float4x4* ShadowLightManager::GetShadowTransformFloat4x4Ptr(D3DTS eState, _uint iIndex)
{
    if (eState < 0 || eState >= D3DTS_END)
        return nullptr;

    auto& list = m_pTransformationMatrices[eState];
    if (iIndex >= list.size())
        return nullptr;

    auto it = list.begin();
    std::advance(it, iIndex);
    return *it;
}

const _float3* ShadowLightManager::GetShadowLightPositionPtr(_uint iIndex)
{
    auto& list = m_vLightPosition;
    if (iIndex >= list.size())
        return nullptr;

    auto it = list.begin();
    std::advance(it, iIndex);
    return *it;
}

const _float* ShadowLightManager::GetShadowLightFarDistancePtr(_uint iIndex)
{
    auto& list = m_fFarDistance;
    if (iIndex >= list.size())
        return nullptr;

    auto it = list.begin();
    std::advance(it, iIndex);
    return *it;
}

void ShadowLightManager::ClearShadowLights()
{
    for (int i = 0; i < D3DTS_END; ++i) {
        for (auto& pTransformMatrix : m_pTransformationMatrices[i])
            SafeDelete(pTransformMatrix);

        m_pTransformationMatrices[i].clear();
    }

    for (auto& pLightPos : m_vLightPosition)
    {
        SafeDelete(pLightPos);
    }
    m_vLightPosition.clear();

    for (auto& fFarDistance : m_fFarDistance)
    {
        SafeDelete(fFarDistance);
    }
    m_fFarDistance.clear();
}

_uint ShadowLightManager::GetNumShadowLights()
{
    return static_cast<_uint>(m_pTransformationMatrices->size());
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
