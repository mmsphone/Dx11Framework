#include "PickingManager.h"

#include "EngineUtility.h"

#include "Object.h"
#include "Terrain.h"
#include "Collision.h"

PickingManager::PickingManager()
    : Base{}, m_pEngineUtility(EngineUtility::GetInstance())
{
    SafeAddRef(m_pEngineUtility);
}

_float3 PickingManager::GetRayHitPosition(const RAY& ray, Object* pObject) const
{
    if (!pObject)
        return _float3(0.f, 0.f, 0.f);

    Terrain* pTerrain = dynamic_cast<Terrain*>(pObject);
    if (pTerrain != nullptr)
    {
        if (RayIntersectTerrain(ray, pTerrain))
        {
            float t = 0.f;
            _float3 rayOrig = ray.origin;
            _float3 rayDir = ray.direction;
            const float step = 0.1f;

            VIBufferTerrain* pVIBufferTerrain = dynamic_cast<VIBufferTerrain*>(pTerrain->FindComponent(TEXT("VIBuffer")));
            if (pVIBufferTerrain == nullptr)
                return _float3(0.f, 0.f, 0.f);

            const auto terrainSizeX = pVIBufferTerrain->GetNumVerticesX();
            const auto terrainSizeZ = pVIBufferTerrain->GetNumVerticesZ();;
            
            for (t = 0.f; t < 1000.f; t += step)
            {
                _float3 pos = _float3(rayOrig.x + rayDir.x * t, rayOrig.y + rayDir.y * t, rayOrig.z + rayDir.z * t);
                if (pos.x < 0 || pos.x >= terrainSizeX || pos.z < 0 || pos.z >= terrainSizeZ)
                    continue;

                float terrainY = pVIBufferTerrain->GetHeightAt(pos.x, pos.z);
                if (pos.y <= terrainY)
                    return _float3(pos.x, terrainY, pos.z);
            }
        }
    }
    else
    {
        if (RayIntersectObject(ray, pObject))
        {
            XMVECTOR origin = XMLoadFloat3(&ray.origin);
            XMVECTOR dir = XMLoadFloat3(&ray.direction);

            Collision* pCollision = dynamic_cast<Collision*>(pObject->FindComponent(TEXT("Collision")));
            if (pCollision == nullptr)
                return _float3(0.f, 0.f, 0.f);

            BoundingBox* pBox = static_cast<BoundingBox*>(pCollision->GetWorldCollisionBox(AABB));
            if (pBox == nullptr)
                return _float3(0.f, 0.f, 0.f);

            float dist = 0.f;
            if (pBox->Intersects(origin, dir, dist))
            {
                XMVECTOR hitPos = origin + dir * dist;
                _float3 result;
                XMStoreFloat3(&result, hitPos);
                return result;
            }
        }
    }

    return _float3(0.f, 0.f, 0.f);
}

_bool PickingManager::RayIntersectObject(const RAY& ray, Object* pObject) const
{
    if (!pObject) 
        return false;

    Collision* pCollision = dynamic_cast<Collision*>(pObject->FindComponent(TEXT("Collision")));
    if (pCollision == nullptr) 
        return false;

    BoundingBox* pBox = static_cast<BoundingBox*>(pCollision->GetWorldCollisionBox(AABB));
    if (pBox == nullptr)
        return false;

    _vector origin = XMLoadFloat3(&ray.origin);
    _vector dir = XMLoadFloat3(&ray.direction);

    float dist = 0.0f;
    return pBox->Intersects(origin, dir, dist);
}

_bool PickingManager::RayIntersectTerrain(const RAY& ray, Terrain* pTerrain) const
{
    if (!pTerrain) return false;

    float t = 0.f;
    _float3 rayOrig = ray.origin;
    _float3 rayDir = ray.direction;

    VIBufferTerrain* pVIBufferTerrain = dynamic_cast<VIBufferTerrain*>(pTerrain->FindComponent(TEXT("VIBuffer")));
    if (pVIBufferTerrain == nullptr)
        return false;

    const auto& heightMap = pVIBufferTerrain->GetHeightMap(); // Terrain 내부에서 제공
    const auto terrainSizeX = pVIBufferTerrain->GetNumVerticesX();
    const auto terrainSizeZ = pVIBufferTerrain->GetNumVerticesZ();

    // 단순 레이 스텝 방식
    const float step = 0.1f;
    for (t = 0.f; t < 1000.f; t += step)
    {
        _float3 pos = _float3(rayOrig.x + rayDir.x * t, rayOrig.y + rayDir.y * t, rayOrig.z + rayDir.z * t);
        if (pos.x < 0 || pos.x >= terrainSizeX || pos.z < 0 || pos.z >= terrainSizeZ)
            continue;

        float terrainY = pVIBufferTerrain->GetHeightAt(pos.x, pos.z);
        if (pos.y <= terrainY)
            return true;
    }
    return false;
}

RAY PickingManager::GetRay()
{
    RAY ray = {};
    _float2 mousePos = m_pEngineUtility->GetMousePos();
    _float2 winSize = m_pEngineUtility->GetWindowSize();
    if (winSize.y <= 0) winSize.y = 1;

    float ndcX = (2.0f * mousePos.x / winSize.x - 1.0f);
    float ndcY = (1.0f - 2.0f * mousePos.y / winSize.y);

    // View Ray
    _vector rayOriginView = XMVectorSet(0.f, 0.f, 0.f, 1.f);
    _vector rayFarView = XMVectorSet(ndcX, ndcY, 1.f, 1.f);

    // Viewport -> ViewSpace 
    _matrix invProjection = m_pEngineUtility->GetTransformMatrixInverse(D3DTS::PROJECTION);
    rayOriginView = XMVector3TransformCoord(rayOriginView, invProjection);
    rayFarView = XMVector3TransformCoord(rayFarView, invProjection);

    // ViewSpace -> World
    _matrix invView = m_pEngineUtility->GetTransformMatrixInverse(D3DTS::VIEW);
    _vector rayOriginWorld = XMVector3TransformCoord(rayOriginView, invView);
    _vector rayFarWorld = XMVector3TransformCoord(rayFarView, invView);

    XMStoreFloat3(&ray.origin, rayOriginWorld);
    XMStoreFloat3(&ray.direction, XMVector3Normalize(rayFarWorld - rayOriginWorld));

    return ray;
}

PickingManager* PickingManager::Create()
{
    return new PickingManager();
}

void PickingManager::Free()
{
    __super::Free();

    SafeRelease(m_pEngineUtility);
}
