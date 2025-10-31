#include "ContainerTemplate.h"

ContainerTemplate::ContainerTemplate()
    :Container{}
{
}

ContainerTemplate::ContainerTemplate(const ContainerTemplate& Prototype)
    :Container{ Prototype }
{
}

HRESULT ContainerTemplate::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT ContainerTemplate::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    if (FAILED(SetUpParts()))
        return E_FAIL;

    return S_OK;
}

void ContainerTemplate::Free()
{
    __super::Free();
}
