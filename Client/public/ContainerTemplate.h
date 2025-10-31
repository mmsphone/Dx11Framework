#pragma once

#include "Client_Defines.h"
#include "Container.h"

NS_BEGIN(Client)

class ContainerTemplate abstract : public Container
{
public:
    ContainerTemplate();
    ContainerTemplate(const ContainerTemplate& Prototype);
    virtual ~ContainerTemplate() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual HRESULT SetUpParts() = 0;

    virtual void Free() override;

protected:
    virtual HRESULT ReadyComponents() = 0;
    virtual HRESULT Render() = 0;

};

NS_END