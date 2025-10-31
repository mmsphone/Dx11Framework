#include "ObjectTemplate.h"

ObjectTemplate::ObjectTemplate()
    :Object{}
{
}

ObjectTemplate::ObjectTemplate(const ObjectTemplate& Prototype)
    :Object{ Prototype }
{
}

HRESULT ObjectTemplate::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT ObjectTemplate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void ObjectTemplate::Free()
{
    __super::Free();
}
