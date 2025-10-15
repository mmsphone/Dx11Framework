#include "CollisionBoxAABB.h"

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
    m_pOriginalDesc->Transform(*m_pDesc, WorldMatrix);
}

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
