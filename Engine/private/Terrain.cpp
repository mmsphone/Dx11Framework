#include "Terrain.h"

#include "EngineUtility.h"

Terrain::Terrain()
    : Object{}
{
}

Terrain::Terrain(const Terrain& Prototype)
    : Object{Prototype}
{
}

HRESULT Terrain::InitializePrototype()
{
    return S_OK;
}

HRESULT Terrain::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
}

void Terrain::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void Terrain::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void Terrain::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT Terrain::Render()
{
    return S_OK;
}

void Terrain::Free()
{
	__super::Free();
}