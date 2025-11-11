#include "Projectile.h"
#include "Transform.h"
#include "EngineUtility.h"

Projectile::Projectile()
    : Object{}
{
}

Projectile::Projectile(const Projectile& Prototype)
    : Object{ Prototype }
{
}

HRESULT Projectile::InitializePrototype()
{
    return S_OK;
}

HRESULT Projectile::Initialize(void* pArg)
{
    if (pArg == nullptr)
        return S_OK;

    m_desc = *static_cast<PROJECTILE_DESC*>(pArg);

    if (FAILED(__super::Initialize(&m_desc)))
        return E_FAIL;

    return S_OK;
}

void Projectile::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void Projectile::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    m_desc.accTime += fTimeDelta;
    if (m_desc.accTime >= m_desc.lifeTime) {
        SetDead(true);
        return;
    }

    auto* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (tf) {
        _vector dir = m_desc.moveDir;
        tf->Translate(dir, fTimeDelta);
    }
}

void Projectile::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT Projectile::Render()
{
    return S_OK;
}

void Projectile::Free()
{
    __super::Free();
}
