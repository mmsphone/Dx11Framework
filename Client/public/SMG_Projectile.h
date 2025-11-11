#pragma once

#include "Client_Defines.h"
#include "Projectile.h"

NS_BEGIN(Client)

class SMG_Projectile final : public Projectile
{
private:
    SMG_Projectile();
    SMG_Projectile(const SMG_Projectile& Prototype);
    virtual ~SMG_Projectile() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static  SMG_Projectile* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void            Free() override;

private:
    HRESULT ReadyComponents();
    _bool TryApplyHitTo(Object* pTarget);
    void HitProjectile(const _vector& projectilePos, _float& fHitRadius, vector<Object*>& out);
};

NS_END