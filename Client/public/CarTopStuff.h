#pragma once
#include "Client_Defines.h"
#include "PartTemplate.h"

NS_BEGIN(Client)

class CarTopStuff final : public PartTemplate
{
private:
    CarTopStuff();
    CarTopStuff(const CarTopStuff& Prototype);
    virtual ~CarTopStuff() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static CarTopStuff* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END