#pragma once

#include "Client_Defines.h"
#include "UI.h"

NS_BEGIN(Client)

class UITemplate abstract : public UI
{
public:
    UITemplate();
    UITemplate(const UITemplate& Prototype);
    virtual ~UITemplate() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    void LateUpdate(_float fTimeDelta) override;

protected:
    virtual HRESULT ReadyComponents() = 0;
    virtual HRESULT Render() = 0;
};

NS_END