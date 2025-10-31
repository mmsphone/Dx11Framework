#include "PartTemplate.h"

PartTemplate::PartTemplate()
    :Part{}
{
}

PartTemplate::PartTemplate(const PartTemplate& Prototype)
    :Part{ Prototype }
{
}

HRESULT PartTemplate::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT PartTemplate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void PartTemplate::Free()
{
    __super::Free();
}
