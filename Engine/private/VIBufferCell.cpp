#include "VIBufferCell.h"

#include "EngineUtility.h"

VIBufferCell::VIBufferCell()
    : VertexIndexBuffer{  }
{
}

VIBufferCell::VIBufferCell(const VIBufferCell& Prototype)
    : VertexIndexBuffer{ Prototype }
{
}

HRESULT VIBufferCell::InitializePrototype(const _float3* pPoints)
{
    m_iNumVertexBuffers = 1;
    m_iNumVertices = 3;
    m_iVertexStride = sizeof(VTXPOS);

    m_iNumIndices = 4;
    m_iIndexStride = 2;
    m_eIndexFormat = m_iIndexStride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    m_ePrimitive = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

    //Vertex Buffer
    D3D11_BUFFER_DESC           VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

    memcpy(pVertices, pPoints, sizeof(_float3) * 3);

    D3D11_SUBRESOURCE_DATA      VertexInitialData{};
    VertexInitialData.pSysMem = pVertices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pVertices);

    //Index Buffer 
    D3D11_BUFFER_DESC           IBDesc{};
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = m_iIndexStride;

    _ushort* pIndices = new _ushort[m_iNumIndices];
    ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

    pIndices[0] = 0;
    pIndices[1] = 1;
    pIndices[2] = 2;
    pIndices[3] = 0;

    D3D11_SUBRESOURCE_DATA      IndexInitialData{};
    IndexInitialData.pSysMem = pIndices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&IBDesc, &IndexInitialData, &m_pIndexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pIndices);

    return S_OK;
}

HRESULT VIBufferCell::Initialize(void* pArg)
{
    return S_OK;
}

VIBufferCell* VIBufferCell::Create(const _float3* pPoints)
{
    VIBufferCell* pInstance = new VIBufferCell();

    if (FAILED(pInstance->InitializePrototype(pPoints)))
    {
        MSG_BOX("Failed to Created : VIBufferCell");
        SafeRelease(pInstance);
    }
    return pInstance;
}


Component* VIBufferCell::Clone(void* pArg)
{
    VIBufferCell* pInstance = new VIBufferCell(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : VIBufferCell");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void VIBufferCell::Free()
{
    __super::Free();
}
