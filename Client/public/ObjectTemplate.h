#pragma once

#include "Client_Defines.h"
#include "Object.h"

NS_BEGIN(Client)

class ObjectTemplate abstract : public Object
{
public:
    ObjectTemplate();
    ObjectTemplate(const ObjectTemplate& Prototype);
    virtual ~ObjectTemplate() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual void Free() override;

protected:
    virtual HRESULT ReadyComponents() = 0;
    virtual HRESULT Render() = 0;
};

NS_END