#pragma once
#include "Client_Defines.h"
#include "PartTemplate.h"

NS_BEGIN(Client)

class CarWheel final : public PartTemplate
{
private:
    CarWheel();
    CarWheel(const CarWheel& Prototype);
    virtual ~CarWheel() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static CarWheel* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END