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
    {
        result.hit = true;
        result.pickType = PICKTYPE_UI;
        return result;
    }

    // --- [A] 기존 레이 기반 피킹 (오브젝트) ---
    _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    vector<Object*> objectList = m_pEngineUtility->GetAllObjects(sceneId);
    float nearestDist = FLT_MAX;
    Object* nearestObj = nullptr;
    _float3 nearestPos{};

    for (auto* obj : objectList)
    {
        _float3 hitPos{};
        if (RayIntersectObject(ray, obj, &hitPos))
        {
            float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&hitPos) - XMLoadFloat3(&ray.origin)));
            if (dist < nearestDist) { nearestDist = dist; nearestObj = obj; nearestPos = hitPos; }
        }
    }

    if (nearestObj) {
        result.hit = true; result.pHitObject = nearestObj; result.hitPos = nearestPos; result.pickType = PICKTYPE_OBJECT;
        return result;
    }

    // --- [B] Terrain 피킹 ---
    if (Layer* pLayer = m_pEngineUtility->FindLayer(sceneId, TEXT("Terrain")))
    {
        for (auto& object : pLayer->GetAllObjects())
        {
            if (auto* pTerrain = dynamic_cast<Terrain*>(object)) {
                _float3 hitPos{};
                if (RayIntersectTerrain(ray, pTerrain, &hitPos)) {
                    result.hit = true; result.pHitObject = pTerrain; result.hitPos = hitPos; result.pickType = PICKTYPE_TERRAIN;
                    return result;
                }
            }
        }
    }

    // --- [C] 픽셀 깊이 기반 피킹 ---
    {
        _float2 mouse = m_pEngineUtility->GetMousePos();
        float d01 = 1.f;

        if (m_pEngineUtility->ReadDepthAtPixel((int)mouse.x, (int)mouse.y, &d01))
        {
            // RS Viewport 가져오기
            D3D11_VIEWPORT vp{}; 
            UINT vpCount = 1;
            m_pEngineUtility->GetContext()->RSGetViewports(&vpCount, &vp);

            // 픽셀센터
            float sx = std::clamp(mouse.x, 0.0f, vp.Width - 1.0f) + 0.5f;
            float sy = std::clamp(mouse.y, 0.0f, vp.Height - 1.0f) + 0.5f;

            // (현재 사용 중인 행렬 사용 — 필요시 LastVP로 교체 가능)
            _matrix V = m_pEngineUtility->GetTransformMatrix(D3DTS_VIEW);
            _matrix P = m_pEngineUtility->GetTransformMatrix(D3DTS_PROJECTION);

            XMVECTOR screen = XMVectorSet(sx, sy, d01, 1.f);
            XMVECTOR wpos = XMVector3Unproject(
                screen,
                vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
                0.0f, 1.0f,
                P, V, XMMatrixIdentity()
            );

            XMStoreFloat3(&result.hitPos, wpos);
            result.hit = true;
            result.pHitObject = nullptr;
            result.pickType = PICKTYPE_PIXEL;

            m_pEngineUtility->SetMarkerPosition(result.hitPos);
            return result;
        }
    }

    // --- [D] Grid(Y=0 평면) 폴백 ---
    {
        _vector o = XMLoadFloat3(&ray.origin);
        _vector d = XMLoadFloat3(&ray.direction);
        _vector n = XMVectorSet(0.f, 1.f, 0.f, 0.f);
        float   fdot = XMVectorGetX(XMVector3Dot(n, d));
        if (fabs(fdot) > 1e-6f) {
            float t = -XMVectorGetX(XMVector3Dot(n, o)) / fdot;
            if (t > 0.f) {
                XMStoreFloat3(&result.hitPos, o + d * t);
                result.hit = true; result.pHitObject = nullptr; result.pickType = PICKTYPE_GRID;
                m_pEngineUtility->SetMarkerPosition(result.hitPos);
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
    case COLLISIONTYPE::COLLISIONTYPE_AABB:
    {
        BoundingBox* pBox = static_cast<BoundingBox*>(pCollision->GetWorldCollisionBox(COLLISIONTYPE_AABB));
        if (pBox && pBox->Intersects(origin, dir, dist) && dist >= 0.f)
            hit = true;
        break;
    }
    case COLLISIONTYPE::COLLISIONTYPE_OBB:
    {
        BoundingOrientedBox* pBox = static_cast<BoundingOrientedBox*>(pCollision->GetWorldCollisionBox(COLLISIONTYPE_OBB));
        if (pBox && pBox->Intersects(origin, dir, dist) && dist >= 0.f)
            hit = true;
        break;
    }
    case COLLISIONTYPE::COLLISIONTYPE_SPHERE:
    {
        BoundingSphere* pBox = static_cast<BoundingSphere*>(pCollision->GetWorldCollisionBox(COLLISIONTYPE_SPHERE));
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
    _matrix invProjection = m_pEngineUtility->GetTransformMatrixInverse(D3DTS::D3DTS_PROJECTION);
    rayOriginView = XMVector3TransformCoord(rayOriginView, invProjection);
    rayFarView = XMVector3TransformCoord(rayFarView, invProjection);

    // ViewSpace -> World
    _matrix invView = m_pEngineUtility->GetTransformMatrixInverse(D3DTS::D3DTS_VIEW);
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

_float3 PickingManager::Unproject(_float depth01, _float mouseX, _float mouseY, _float winW, _float winH, const _float4x4& invViewProj)
{
    _float ndcX = 2.f * (mouseX / winW) - 1.f;
    _float ndcY = -2.f * (mouseY / winH) + 1.f;
    _float clipZ = depth01 * 2.f - 1.f;

    _vector clip = XMVectorSet(ndcX, ndcY, clipZ, 1.f);
    _matrix inv = XMLoadFloat4x4(&invViewProj);
    _vector wpos = XMVector4Transform(clip, inv);
    wpos = XMVectorScale(wpos, 1.f / XMVectorGetW(wpos));

    _float3 out; XMStoreFloat3(&out, wpos);
    return out;
}
