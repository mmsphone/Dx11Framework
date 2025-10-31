#pragma once

#include "Client_Defines.h"
#include "ContainerTemplate.h"

NS_BEGIN(Client)

class HouseDoor final : public ContainerTemplate
{
private:
    HouseDoor();
    HouseDoor(const HouseDoor& Prototype);
    virtual ~HouseDoor() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual HRESULT SetUpParts() override;

    static HouseDoor* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END