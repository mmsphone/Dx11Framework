#pragma once
#include "Client_Defines.h"
#include "PartTemplate.h"

NS_BEGIN(Client)

class CarTrunkHandle final : public PartTemplate
{
private:
    CarTrunkHandle();
    CarTrunkHandle(const CarTrunkHandle& Prototype);
    virtual ~CarTrunkHandle() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static CarTrunkHandle* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END