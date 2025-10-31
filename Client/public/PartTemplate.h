#pragma once

#include "Client_Defines.h"
#include "Part.h"

NS_BEGIN(Client)

class PartTemplate abstract : public Part
{
public:
    PartTemplate();
    PartTemplate(const PartTemplate& Prototype);
    virtual ~PartTemplate() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual void Free() override;

protected:
    virtual HRESULT ReadyComponents() = 0;
    virtual HRESULT Render() = 0;
};

NS_END