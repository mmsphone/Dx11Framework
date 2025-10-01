#include "VIBufferTerrain.h"
#include "EngineUtility.h"

VIBufferTerrain::VIBufferTerrain()
    : VertexIndexBuffer{}
{
}

VIBufferTerrain::VIBufferTerrain(const VIBufferTerrain& Prototype)
    : VertexIndexBuffer{ Prototype }
    , m_iNumVerticesX{ Prototype.m_iNumVerticesX }
    , m_iNumVerticesZ{ Prototype.m_iNumVerticesZ }
{
}

HRESULT VIBufferTerrain::InitializePrototype(const _tchar* pHeightMapFilePath)
{
    _ulong          dwByte = {};
    HANDLE          hFile = CreateFile(pHeightMapFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    BITMAPFILEHEADER            fh{};
    ReadFile(hFile, &fh, sizeof fh, &dwByte, nullptr);

    BITMAPINFOHEADER            ih{};
    ReadFile(hFile, &ih, sizeof ih, &dwByte, nullptr);

    _uint* pPixels = new _uint[ih.biWidth * ih.biHeight];
    ReadFile(hFile, pPixels, sizeof(_uint) * ih.biWidth * ih.biHeight, &dwByte, nullptr);

    CloseHandle(hFile);

    m_iNumVerticesX = ih.biWidth;
    m_iNumVerticesZ = ih.biHeight;

    m_iNumVertexBuffers = 1;
    m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;
    m_iVertexStride = sizeof(VTXNORTEX);

    m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
    m_iIndexStride = 4;
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

    VTXNORTEX* pVertices = new VTXNORTEX[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXNORTEX) * m_iNumVertices);

    for (size_t i = 0; i < m_iNumVerticesZ; i++)
    {
        for (size_t j = 0; j < m_iNumVerticesX; j++)
        {
            _uint       iIndex = i * m_iNumVerticesX + j;

            /* pPixels[iIndex] : 0xff151515 */
            /*               &   0x000000ff */
            /*               &   0x00000015 */

            pVertices[iIndex].vPosition = _float3(j, (pPixels[iIndex] & 0x000000ff) / 10.0f, i);
            pVertices[iIndex].vNormal = _float3(0.0f, 0.0f, 0.f);
            pVertices[iIndex].vTexcoord = _float2(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
        }
    }





#pragma endregion

#pragma region INDEX_BUFFER 
    D3D11_BUFFER_DESC           IBDesc{};
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = m_iIndexStride;

    _uint* pIndices = new _uint[m_iNumIndices];
    ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

    _uint       iNumIndices = {};

    for (size_t i = 0; i < m_iNumVerticesZ - 1; i++)
    {
        for (size_t j = 0; j < m_iNumVerticesX - 1; j++)
        {
            _uint       iIndex = i * m_iNumVerticesX + j;

            _uint       iIndices[4] = {
                iIndex + m_iNumVerticesX,
                iIndex + m_iNumVerticesX + 1,
                iIndex + 1,
                iIndex
            };

            _vector     vSour, vDest, vNormal;

            pIndices[iNumIndices++] = iIndices[0];
            pIndices[iNumIndices++] = iIndices[1];
            pIndices[iNumIndices++] = iIndices[2];

            vSour = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
            vDest = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[1]].vPosition);
            vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

            XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
                XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
            XMStoreFloat3(&pVertices[iIndices[1]].vNormal,
                XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + vNormal);
            XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
                XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);


            pIndices[iNumIndices++] = iIndices[0];
            pIndices[iNumIndices++] = iIndices[2];
            pIndices[iNumIndices++] = iIndices[3];

            vSour = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
            vDest = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) - XMLoadFloat3(&pVertices[iIndices[2]].vPosition);
            vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

            XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
                XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
            XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
                XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
            XMStoreFloat3(&pVertices[iIndices[3]].vNormal,
                XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + vNormal);
        }
    }

    for (size_t i = 0; i < m_iNumVertices; i++)
    {
        XMStoreFloat3(&pVertices[i].vNormal,
            XMVector3Normalize(XMLoadFloat3(&pVertices[i].vNormal)));
    }




#pragma endregion

    D3D11_SUBRESOURCE_DATA      VertexInitialData{};
    VertexInitialData.pSysMem = pVertices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    D3D11_SUBRESOURCE_DATA      IndexInitialData{};
    IndexInitialData.pSysMem = pIndices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&IBDesc, &IndexInitialData, &m_pIndexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pIndices);
    SafeDeleteArray(pVertices);
    SafeDeleteArray(pPixels);

    return S_OK;
}

HRESULT VIBufferTerrain::Initialize(void* pArg)
{
    return S_OK;
}

VIBufferTerrain* VIBufferTerrain::Create(const _tchar* pHeightMapFilePath)
{
    VIBufferTerrain* pInstance = new VIBufferTerrain();

    if (FAILED(pInstance->InitializePrototype(pHeightMapFilePath)))
    {
        MSG_BOX("Failed to Created : VIBufferTerrain");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Component* VIBufferTerrain::Clone(void* pArg)
{
    VIBufferTerrain* pInstance = new VIBufferTerrain(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : VIBufferTerrain");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void VIBufferTerrain::Free()
{
    __super::Free();


}
