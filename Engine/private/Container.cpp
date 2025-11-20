#include "Container.h"
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
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (pArg == nullptr)
        return S_OK;

    CONTAINER_DESC* pDesc = static_cast<CONTAINER_DESC*>(pArg);

    m_iNumParts = pDesc->iNumParts;

    m_Parts.resize(m_iNumParts);


    return S_OK;
}

void Container::PriorityUpdate(_float fTimeDelta)
{
    for (auto& pPartObj : m_Parts)
    {
        if (nullptr != pPartObj)
            pPartObj->PriorityUpdate(fTimeDelta);
    }
}

void Container::Update(_float fTimeDelta)
{
    for (auto& pPartObj : m_Parts)
    {
        if (nullptr != pPartObj)
            pPartObj->Update(fTimeDelta);
    }
}

void Container::LateUpdate(_float fTimeDelta)
{
    for (auto& pPartObj : m_Parts)
    {
        if (nullptr != pPartObj)
            pPartObj->LateUpdate(fTimeDelta);
    }
}

HRESULT Container::Render()
{
    return S_OK;
}

HRESULT Container::AddPart(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iPartObjectIndex, void* pArg)
{
    Part* pPartObject = dynamic_cast<Part*>(m_pEngineUtility->ClonePrototype(PROTOTYPE::PROTOTYPE_OBJECT, iPrototypeLevelID, strPrototypeTag, pArg));
    if (nullptr == pPartObject)
        return E_FAIL;

    m_Parts[iPartObjectIndex] = pPartObject;

    return S_OK;
}

const vector<class Part*>& Container::GetParts()
{
    return m_Parts;
}

void Container::Free()
{
    __super::Free();

    for (auto& pPart : m_Parts)
        SafeRelease(pPart);

    m_Parts.clear();
}
