#include "EffectTemplate.h"

EffectTemplate::EffectTemplate() 
    : Effect{}
{
}

EffectTemplate::EffectTemplate(const EffectTemplate& Prototype)
    : Effect{ Prototype }
{
}

HRESULT EffectTemplate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    OnEffectStart(pArg);

    return S_OK;
}

void EffectTemplate::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
    if (IsDead())
        return;

    OnEffectUpdate(fTimeDelta);

    if (!m_bNotifiedEnd && IsFinished())
    {
        OnEffectEnd();
        m_bNotifiedEnd = true;
    }
}
