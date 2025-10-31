#pragma once

#include "Client_Defines.h"
#include "ContainerTemplate.h"

NS_BEGIN(Client)

class CarBody final : public ContainerTemplate
{
private:
    CarBody();
    CarBody(const CarBody& Prototype);
    virtual ~CarBody() = default;

public:
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual HRESULT SetUpParts() override;

    static CarBody* Create();
    virtual Object* Clone(void* pArg = nullptr) override;

private:
    virtual HRESULT ReadyComponents() override;
};

NS_END