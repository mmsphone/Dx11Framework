#include "Part.h"

Part::Part()
	:Object{}
{
}

Part::Part(const Part& Prototype)
	:Object{ Prototype }
{
}

HRESULT Part::InitializePrototype()
{
	return S_OK;
}

HRESULT Part::Initialize(void* pArg)
{
    PART_DESC* pDesc = static_cast<PART_DESC*>(pArg);
    m_pParentMatrix = pDesc->pParentMatrix;
    XMStoreFloat4x4(&m_CombinedWorldMatrix, XMMatrixIdentity());

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void Part::PriorityUpdate(_float fTimeDelta)
{
}

void Part::Update(_float fTimeDelta)
{
}

void Part::LateUpdate(_float fTimeDelta)
{
}

HRESULT Part::Render()
{
	return S_OK;
}

const _float4x4* Part::GetCombinedWorldMatrix() const
{
    return &m_CombinedWorldMatrix;
}

void Part::Free()
{
    __super::Free();
}
