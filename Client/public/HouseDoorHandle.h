#pragma once
#include "Client_Defines.h"
#include "PartTemplate.h"

NS_BEGIN(Client)

class HouseDoorHandle final : public PartTemplate
{
private:
    HouseDoorHandle();
    HouseDoorHandle(const HouseDoorHandle& Prototype);
    virtual ~HouseDoorHandle() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static HouseDoorHandle* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END