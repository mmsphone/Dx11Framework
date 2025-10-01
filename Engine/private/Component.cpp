#include "Component.h"
#include "EngineUtility.h"

Component::Component()
    : m_pEngineUtility{ EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

Component::Component(const Component& Prototype)
    :m_pEngineUtility{ Prototype.m_pEngineUtility }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT Component::InitializePrototype()
{
    return S_OK;
}

HRESULT Component::Initialize(void* pArg)
{
    return S_OK;
}

void Component::Free()
{
    __super::Free();

    m_pEngineUtility->DestroyInstance();
}
