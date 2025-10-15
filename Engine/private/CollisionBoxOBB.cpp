#include "CollisionBoxOBB.h"

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
