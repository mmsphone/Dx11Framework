#include "CollisionBoxSphere.h"

#include "Collision.h"

CollisionBoxSphere::CollisionBoxSphere()
    : CollisionBox{  }
{
}

HRESULT CollisionBoxSphere::Initialize(CollisionBox::COLLISIONBOX_DESC* pInitialDesc)
{
    COLLISIONSPHERE_DESC* pDesc = static_cast<CollisionBoxSphere::COLLISIONSPHERE_DESC*>(pInitialDesc);

    m_pOriginalDesc = new BoundingSphere(pDesc->vCenter, _float(pDesc->fRadius));
    m_pDesc = new BoundingSphere(*m_pOriginalDesc);

    return S_OK;
}

void CollisionBoxSphere::Update(_fmatrix WorldMatrix)
{
    m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);
}

_bool CollisionBoxSphere::Intersect(Collision* pCollision)
{
    _bool isIntersected = false;

    switch (pCollision->GetType())
    {
    case COLLISIONTYPE::COLLISIONTYPE_AABB:
        isIntersected = m_pDesc->Intersects(*static_cast<BoundingBox*>(pCollision->GetWorldCollisionBox(COLLISIONTYPE_AABB)));
        break;

    case COLLISIONTYPE::COLLISIONTYPE_OBB:
        isIntersected = m_pDesc->Intersects(*static_cast<BoundingOrientedBox*>(pCollision->GetWorldCollisionBox(COLLISIONTYPE_OBB)));
        break;

    case COLLISIONTYPE::COLLISIONTYPE_SPHERE:
        isIntersected = m_pDesc->Intersects(*static_cast<BoundingSphere*>(pCollision->GetWorldCollisionBox(COLLISIONTYPE_SPHERE)));
        break;
    }

    return isIntersected;
}
#ifdef _DEBUG
HRESULT CollisionBoxSphere::Render(PrimitiveBatch<VertexPositionColor>* pBatch, _fvector vColor)
{
    DX::Draw(pBatch, *m_pDesc, vColor);

    return S_OK;
}
#endif
BoundingSphere* CollisionBoxSphere::GetLocalBox() const
{
    return m_pOriginalDesc;
}

BoundingSphere* CollisionBoxSphere::GetWorldBox() const
{
    return m_pDesc;
}

CollisionBoxSphere* CollisionBoxSphere::Create(CollisionBox::COLLISIONBOX_DESC* pInitialDesc)
{
    CollisionBoxSphere* pInstance = new CollisionBoxSphere();

    if (FAILED(pInstance->Initialize(pInitialDesc)))
    {
        MSG_BOX("Failed to Created : CollisionBoxSphere");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void CollisionBoxSphere::Free()
{
    __super::Free();

    SafeDelete(m_pDesc);
    SafeDelete(m_pOriginalDesc);
}
