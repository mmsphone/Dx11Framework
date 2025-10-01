#include "VIBufferRect.h"
#include "EngineUtility.h"

VIBufferRect::VIBufferRect()
    : VertexIndexBuffer{}
{
}

VIBufferRect::VIBufferRect(const VIBufferRect& Prototype)
    : VertexIndexBuffer{ Prototype }
{
}

HRESULT VIBufferRect::InitializePrototype()
{
    m_iNumVertexBuffers = 1;
    m_iNumVertices = 4;
    m_iVertexStride = sizeof(VTXPOSTEX);

    m_iNumIndices = 6;
    m_iIndexStride = 2;
    m_eIndexFormat = m_iIndexStride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    m_ePrimitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER
    D3D11_BUFFER_DESC           VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    VTXPOSTEX* pVertices = new VTXPOSTEX[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXPOSTEX) * m_iNumVertices);

    pVertices[0].vPosition = _float3(-0.5f, 0.5f, 0.f);
    pVertices[0].vTexcoord = _float2(0.f, 0.f);

    pVertices[1].vPosition = _float3(0.5f, 0.5f, 0.f);
    pVertices[1].vTexcoord = _float2(1.f, 0.f);

    pVertices[2].vPosition = _float3(0.5f, -0.5f, 0.f);
    pVertices[2].vTexcoord = _float2(1.f, 1.f);

    pVertices[3].vPosition = _float3(-0.5f, -0.5f, 0.f);
    pVertices[3].vTexcoord = _float2(0.f, 1.f);

    D3D11_SUBRESOURCE_DATA      VertexInitialData{};
    VertexInitialData.pSysMem = pVertices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pVertices);

#pragma endregion

#pragma region INDEX_BUFFER 
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
    pIndices[4] = 2;
    pIndices[5] = 3;

    D3D11_SUBRESOURCE_DATA      IndexInitialData{};
    IndexInitialData.pSysMem = pIndices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&IBDesc, &IndexInitialData, &m_pIndexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pIndices);

#pragma endregion


    return S_OK;
}

HRESULT VIBufferRect::Initialize(void* pArg)
{
    return S_OK;
}

VIBufferRect* VIBufferRect::Create()
{
    VIBufferRect* pInstance = new VIBufferRect();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : VIBufferRect");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Component* VIBufferRect::Clone(void* pArg)
{
    VIBufferRect* pInstance = new VIBufferRect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : VIBufferRect");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void VIBufferRect::Free()
{
    __super::Free();
}
