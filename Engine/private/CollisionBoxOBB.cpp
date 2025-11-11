#include "CollisionBoxOBB.h"

#include "Collision.h"

CollisionBoxOBB::CollisionBoxOBB()
    : CollisionBox{  }
{
}

HRESULT CollisionBoxOBB::Initialize(CollisionBox::COLLISIONBOX_DESC* pInitialDesc)
{
    COLLISIONOBB_DESC* pDesc = static_cast<CollisionBoxOBB::COLLISIONOBB_DESC*>(pInitialDesc);

    m_pOriginalDesc = new BoundingOrientedBox(pDesc->vCenter, _float3(pDesc->vExtents.x,pDesc->vExtents.y,pDesc->vExtents.z), 
        _float4(pDesc->vOrientation.x, pDesc->vOrientation.y, pDesc->vOrientation.z, pDesc->vOrientation.w));
    m_pDesc = new BoundingOrientedBox(*m_pOriginalDesc);

    return S_OK;
}

void CollisionBoxOBB::Update(_fmatrix WorldMatrix)
{
    m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);
}

_bool CollisionBoxOBB::Intersect(Collision* pCollision)
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
HRESULT CollisionBoxOBB::Render(PrimitiveBatch<VertexPositionColor>* pBatch, _fvector vColor)
{
    DX::Draw(pBatch, *m_pDesc, vColor);

    return S_OK;
}
#endif
BoundingOrientedBox* CollisionBoxOBB::GetLocalBox() const
{
    return m_pOriginalDesc;
}

BoundingOrientedBox* CollisionBoxOBB::GetWorldBox() const
{
    return m_pDesc;
}

CollisionBoxOBB* CollisionBoxOBB::Create(CollisionBox::COLLISIONBOX_DESC* pInitialDesc)
{
    CollisionBoxOBB* pInstance = new CollisionBoxOBB();

    if (FAILED(pInstance->Initialize(pInitialDesc)))
    {
        MSG_BOX("Failed to Created : CollisionBoxOBB");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void CollisionBoxOBB::Free()
{
    __super::Free();

    SafeDelete(m_pDesc);
    SafeDelete(m_pOriginalDesc);
}
