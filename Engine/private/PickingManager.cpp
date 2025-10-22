#include "PickingManager.h"

#include "EngineUtility.h"

#include "Object.h"
#include "Terrain.h"
#include "Collision.h"
#include "Layer.h"

PickingManager::PickingManager()
    : Base{}, m_pEngineUtility(EngineUtility::GetInstance())
{
    SafeAddRef(m_pEngineUtility);
}

PICK_RESULT PickingManager::Pick()
{
    PICK_RESULT result{};

    RAY ray = GetRay();

    if (IsMouseOverUI())
        return result;

    _uint iCurrentSceneId = m_pEngineUtility->GetCurrentSceneId();
    vector<Object*> objectList = m_pEngineUtility->GetAllObjects(iCurrentSceneId);
    float nearestDist = FLT_MAX;
    Object* nearestObj = nullptr;
    _float3 nearestPos{};

    for (auto* obj : objectList)
    {
        _float3 hitPos{};
        if (RayIntersectObject(ray, obj, &hitPos))
        {
            float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&hitPos) - XMLoadFloat3(&ray.origin)));
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearestObj = obj;
                nearestPos = hitPos;
            }
        }
    }

    if (nearestObj)
    {
        result.hit = true;
        result.pHitObject = nearestObj;
        result.hitPos = nearestPos;
        return result;
    }

    // 3️⃣ Terrain 피킹 (오브젝트 실패 시)
    list<Object*> objects = m_pEngineUtility->FindLayer(iCurrentSceneId, TEXT("Terrain"))->GetAllObjects();
    for (auto& object : objects)
    {
        Terrain* pTerrain = dynamic_cast<Terrain*>(object);
        if (pTerrain)
        {
            _float3 hitPos{};
            if (RayIntersectTerrain(ray, pTerrain, &hitPos))
            {
                result.hit = true;
                result.pHitObject = pTerrain;
                result.hitPos = hitPos;
                return result;
            }
        }
    }
    
    return result;
}

_bool PickingManager::IsMouseOverUI() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

_bool PickingManager::RayIntersectObject(const RAY& ray, Object* pObject, _float3* pOutHitPos) const
{
    if (!pObject) 
        return false;

    Collision* pCollision = dynamic_cast<Collision*>(pObject->FindComponent(TEXT("Collision")));
    if (pCollision == nullptr) 
        return false;

    _vector origin = XMLoadFloat3(&ray.origin);
    _vector dir = XMLoadFloat3(&ray.direction);
    _float dist = 0.0f;
    _bool hit = false;

    switch (pCollision->GetType())
    {
    case COLLISIONTYPE::AABB:
    {
        BoundingBox* pBox = static_cast<BoundingBox*>(pCollision->GetWorldCollisionBox(AABB));
        if (pBox && pBox->Intersects(origin, dir, dist) && dist >= 0.f)
            hit = true;
        break;
    }
    case COLLISIONTYPE::OBB:
    {
        BoundingOrientedBox* pBox = static_cast<BoundingOrientedBox*>(pCollision->GetWorldCollisionBox(OBB));
        if (pBox && pBox->Intersects(origin, dir, dist) && dist >= 0.f)
            hit = true;
        break;
    }
    case COLLISIONTYPE::SPHERE:
    {
        BoundingSphere* pBox = static_cast<BoundingSphere*>(pCollision->GetWorldCollisionBox(SPHERE));
        if (pBox && pBox->Intersects(origin, dir, dist) && dist >= 0.f)
            hit = true;
        break;
    }
    }


    if (hit && pOutHitPos)
    {
        XMVECTOR hitPos = origin + dir * dist;
        XMStoreFloat3(pOutHitPos, hitPos);
    }
    return hit;
}

_bool PickingManager::RayIntersectTerrain(const RAY& ray, Terrain* pTerrain, _float3* pOutHitPos) const
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
        {
            if (pOutHitPos)
                *pOutHitPos = _float3(pos.x, terrainY, pos.z);
            return true;
        }
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
