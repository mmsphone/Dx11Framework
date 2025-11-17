#pragma once

#include "Client_Defines.h"
#include "Effect.h"

NS_BEGIN(Client)

class EffectTemplate abstract : public Effect
{
protected:
    EffectTemplate();
    EffectTemplate(const EffectTemplate& Prototype);
    virtual ~EffectTemplate() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;

protected:
    virtual HRESULT ReadyComponents() = 0;
    virtual void    OnEffectStart(void* pArg) = 0;
    virtual void    OnEffectUpdate(_float fTimeDelta) = 0;
    virtual void    OnEffectEnd() = 0;

protected:
    _bool m_bNotifiedEnd = false;
};

NS_END