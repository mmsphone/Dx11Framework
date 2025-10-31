#pragma once
#include "Client_Defines.h"
#include "PartTemplate.h"

NS_BEGIN(Client)

class CarTrunk final : public PartTemplate
{
private:
    CarTrunk();
    CarTrunk(const CarTrunk& Prototype);
    virtual ~CarTrunk() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static CarTrunk* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END