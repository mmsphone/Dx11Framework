#pragma once

#include "Client_Defines.h"
#include "Projectile.h"

NS_BEGIN(Client)

class Worm_Projectile final : public Projectile
{
private:
    Worm_Projectile();
    Worm_Projectile(const Worm_Projectile& Prototype);
    virtual ~Worm_Projectile() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static  Worm_Projectile* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void            Free() override;

private:
    HRESULT ReadyComponents();
    _bool TryApplyHitTo(Object* pTarget);
    void HitProjectile(vector<Object*>& out);
};

NS_END