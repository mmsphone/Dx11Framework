#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Shieldbug final : public ObjectTemplate
{
private:
    Shieldbug();
    Shieldbug(const Shieldbug& Prototype);
    virtual ~Shieldbug() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static Shieldbug* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void Free() override;

private:
    HRESULT ReadyComponents();
};

NS_END
