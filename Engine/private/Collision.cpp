#include "Collision.h"

Collision::Collision(COLLISIONTYPE eType)
    : Component{}
    ,m_CollisionType{ eType }
{

}

Collision::Collision(const Collision& Prototype)
    : Component{ Prototype }
    , m_CollisionType{ Prototype.m_CollisionType }

{

}

HRESULT Collision::Initialize(void* pArg)
{
    CollisionBox::COLLISIONBOX_DESC* pDesc = static_cast<CollisionBox::COLLISIONBOX_DESC*>(pArg);
    switch (m_CollisionType)
    {
    case COLLISIONTYPE::AABB:
        m_pCollisionBox = CollisionBoxAABB::Create(pDesc);
        break;
    case COLLISIONTYPE::OBB:
        m_pCollisionBox = CollisionBoxOBB::Create(pDesc);
        break;
    case COLLISIONTYPE::SPHERE:
        m_pCollisionBox = CollisionBoxSphere::Create(pDesc);
        break;
    }

    return S_OK;
}

COLLISIONTYPE Collision::GetType() const
{
    return m_CollisionType;
}

void* Collision::GetWorldCollisionBox(COLLISIONTYPE eType)
{
    switch (eType)
    {
    case COLLISIONTYPE::AABB:
        return dynamic_cast<CollisionBoxAABB*>(m_pCollisionBox)->GetWorldBox();
    case COLLISIONTYPE::OBB:
        return dynamic_cast<CollisionBoxOBB*>(m_pCollisionBox)->GetWorldBox();
    case COLLISIONTYPE::SPHERE:
        return dynamic_cast<CollisionBoxSphere*>(m_pCollisionBox)->GetWorldBox();
    }
    return nullptr;
}

void* Collision::GetLocalCollisionBox(COLLISIONTYPE eType)
{
    switch (eType)
    {
    case COLLISIONTYPE::AABB:
        return dynamic_cast<CollisionBoxAABB*>(m_pCollisionBox)->GetLocalBox();
    case COLLISIONTYPE::OBB:
        return dynamic_cast<CollisionBoxOBB*>(m_pCollisionBox)->GetLocalBox();
    case COLLISIONTYPE::SPHERE:
        return dynamic_cast<CollisionBoxSphere*>(m_pCollisionBox)->GetLocalBox();
    }
    return nullptr;
}

Collision* Collision::Create(COLLISIONTYPE eType)
{
    return new Collision(eType);
}

Component* Collision::Clone(void* pArg)
{
    Collision* pInstance = new Collision(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Collision");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Collision::Free()
{
    __super::Free();

    SafeRelease(m_pCollisionBox);


}
