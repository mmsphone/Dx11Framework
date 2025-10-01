﻿#include "Container.h"
#include "EngineUtility.h"
#include "Part.h"

Container::Container()
    :Object{}
{
}

Container::Container(const Container& Prototype)
    :Object{ Prototype}
{
}

HRESULT Container::InitializePrototype()
{
    return S_OK;
}

HRESULT Container::Initialize(void* pArg)
{
    CONTAINER_DESC* pDesc = static_cast<CONTAINER_DESC*>(pArg);

    m_iNumParts = pDesc->iNumParts;

    m_Parts.resize(m_iNumParts);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void Container::PriorityUpdate(_float fTimeDelta)
{
}

void Container::Update(_float fTimeDelta)
{
}

void Container::LateUpdate(_float fTimeDelta)
{
}

HRESULT Container::Render()
{
    return S_OK;
}

void Container::Free()
{
    __super::Free();

    for (auto& pPart : m_Parts)
        SafeRelease(pPart);

    m_Parts.clear();
}

HRESULT Container::AddPart(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iPartObjectIndex, void* pArg)
{
    Part* pPartObject = dynamic_cast<Part*>(m_pEngineUtility->ClonePrototype(PROTOTYPE::OBJECT, iPrototypeLevelID, strPrototypeTag, pArg));
    if (nullptr == pPartObject)
        return E_FAIL;

    m_Parts[iPartObjectIndex] = pPartObject;

    return S_OK;
}
