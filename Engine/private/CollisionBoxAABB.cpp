#include "CollisionBoxAABB.h"

#include "Collision.h"

CollisionBoxAABB::CollisionBoxAABB()
    : CollisionBox{  }
{
}

HRESULT CollisionBoxAABB::Initialize(CollisionBox::COLLISIONBOX_DESC* pInitialDesc)
{
    COLLISIONAABB_DESC* pDesc = static_cast<CollisionBoxAABB::COLLISIONAABB_DESC*>(pInitialDesc);

    m_pOriginalDesc = new BoundingBox(pDesc->vCenter, _float3(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f));
    m_pDesc = new BoundingBox(*m_pOriginalDesc);

    return S_OK;
}

void CollisionBoxAABB::Update(_fmatrix WorldMatrix)
{
    _matrix     TransformMatrix = WorldMatrix;

    TransformMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(WorldMatrix.r[0]);
    TransformMatrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(WorldMatrix.r[1]);
    TransformMatrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(WorldMatrix.r[2]);

    m_pOriginalDesc->Transform(*m_pDesc, TransformMatrix);
}

_bool CollisionBoxAABB::Intersect(Collision* pCollision)
{
    _bool isIntersected = false;

    switch (pCollision->GetType())
    {
    case COLLISIONTYPE::AABB:
        isIntersected = m_pDesc->Intersects(*static_cast<BoundingBox*>(pCollision->GetWorldCollisionBox(AABB)));
        break;

    case COLLISIONTYPE::OBB:
        isIntersected = m_pDesc->Intersects(*static_cast<BoundingOrientedBox*>(pCollision->GetWorldCollisionBox(OBB)));
        break;

    case COLLISIONTYPE::SPHERE:
        isIntersected = m_pDesc->Intersects(*static_cast<BoundingSphere*>(pCollision->GetWorldCollisionBox(SPHERE)));
        break;
    }

    return isIntersected;
}
#ifdef _DEBUG
HRESULT CollisionBoxAABB::Render(PrimitiveBatch<VertexPositionColor>* pBatch, _fvector vColor)
{
    DX::Draw(pBatch, *m_pDesc, vColor);

    return S_OK;
}
#endif
BoundingBox* CollisionBoxAABB::GetLocalBox() const
{
    return m_pOriginalDesc;
}

BoundingBox* CollisionBoxAABB::GetWorldBox() const
{
    return m_pDesc;
}

CollisionBoxAABB* CollisionBoxAABB::Create(CollisionBox::COLLISIONBOX_DESC* pInitialDesc)
{
    CollisionBoxAABB* pInstance = new CollisionBoxAABB();

    if (FAILED(pInstance->Initialize(pInitialDesc)))
    {
        MSG_BOX("Failed to Created : CollisionBoxAABB");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void CollisionBoxAABB::Free()
{
    __super::Free();

    SafeDelete(m_pDesc);
    SafeDelete(m_pOriginalDesc);
}
