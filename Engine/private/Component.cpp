﻿#include "Component.h"
#include "EngineUtility.h"

Component::Component()
    : m_pEngineUtility{ EngineUtility::GetInstance() }
    , m_isCloned{ false }
{
    SafeAddRef(m_pEngineUtility);
}

Component::Component(const Component& Prototype)
    :m_pEngineUtility{ Prototype.m_pEngineUtility }
    ,m_isCloned{ true }
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
    SafeRelease(m_pEngineUtility);
}
