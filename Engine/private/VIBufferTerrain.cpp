#include "VIBufferTerrain.h"
#include "EngineUtility.h"

VIBufferTerrain::VIBufferTerrain()
    : VertexIndexBuffer{}
{
}

VIBufferTerrain::VIBufferTerrain(const VIBufferTerrain& Prototype)
    : VertexIndexBuffer{ Prototype }
    , m_HeightMapData{ Prototype.m_HeightMapData }
    , m_iNumVerticesX{ Prototype.m_iNumVerticesX }
    , m_iNumVerticesZ{ Prototype.m_iNumVerticesZ }
{
}

HRESULT VIBufferTerrain::InitializePrototype(const _tchar* pHeightMapFilePath)
{
    // Load HeightMap
    if (FAILED(LoadHeightMap(pHeightMapFilePath)))
        return E_FAIL;

    m_iNumVertexBuffers = 1;
    m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;
    m_iVertexStride = sizeof(VTXNORTEX);

    m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
    m_iIndexStride = 4;
    m_eIndexFormat = m_iIndexStride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    m_ePrimitive = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    //Vertex Buffer
    D3D11_BUFFER_DESC VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    VTXNORTEX* pVertices = new VTXNORTEX[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXNORTEX) * m_iNumVertices);

    for (_uint z = 0; z < m_iNumVerticesZ; ++z)
    {
        for (_uint x = 0; x < m_iNumVerticesX; ++x)
        {
            size_t iIndex = z * m_iNumVerticesX + x;
            pVertices[iIndex].vPosition = _float3((float)x, m_HeightMapData[z][x], (float)z);
            pVertices[iIndex].vNormal = _float3(0.f, 1.f, 0.f);
            pVertices[iIndex].vTexcoord = _float2(x / (m_iNumVerticesX - 1.f), z / (m_iNumVerticesZ - 1.f));
        }
    }

    //Index Buffer
    D3D11_BUFFER_DESC IBDesc{};
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.StructureByteStride = m_iIndexStride;

    _uint* pIndices = new _uint[m_iNumIndices];
    ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

    _uint iNumIndices = 0;

    for (_uint z = 0; z < m_iNumVerticesZ - 1; ++z)
    {
        for (_uint x = 0; x < m_iNumVerticesX - 1; ++x)
        {
            _uint iIndex = z * m_iNumVerticesX + x;

            _uint iIndices[4] = {
                iIndex + m_iNumVerticesX,
                iIndex + m_iNumVerticesX + 1,
                iIndex + 1,
                iIndex
            };

            // 첫 번째 삼각형
            pIndices[iNumIndices++] = iIndices[0];
            pIndices[iNumIndices++] = iIndices[1];
            pIndices[iNumIndices++] = iIndices[2];

            // 두 번째 삼각형
            pIndices[iNumIndices++] = iIndices[0];
            pIndices[iNumIndices++] = iIndices[2];
            pIndices[iNumIndices++] = iIndices[3];
        }
    }

    D3D11_SUBRESOURCE_DATA VertexInitialData{};
    VertexInitialData.pSysMem = pVertices;
    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    D3D11_SUBRESOURCE_DATA IndexInitialData{};
    IndexInitialData.pSysMem = pIndices;
    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&IBDesc, &IndexInitialData, &m_pIndexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pIndices);
    SafeDeleteArray(pVertices);

    return S_OK;
}
HRESULT VIBufferTerrain::Initialize(void* pArg)
{
    return S_OK;
}

float VIBufferTerrain::GetHeightAt(float x, float z) const
{
    if (x < 0 || z < 0 || x >= m_iNumVerticesX - 1 || z >= m_iNumVerticesZ - 1)
        return 0.f;

    // bilinear interpolation
    int ix = static_cast<int>(x);
    int iz = static_cast<int>(z);

    float fracX = x - ix;
    float fracZ = z - iz;

    float h00 = m_HeightMapData[iz][ix];
    float h10 = m_HeightMapData[iz][ix + 1];
    float h01 = m_HeightMapData[iz + 1][ix];
    float h11 = m_HeightMapData[iz + 1][ix + 1];

    float h0 = h00 + (h10 - h00) * fracX;
    float h1 = h01 + (h11 - h01) * fracX;
    float h = h0 + (h1 - h0) * fracZ;

    return h;

}

const vector<vector<float>>& VIBufferTerrain::GetHeightMap() const
{
    return m_HeightMapData;
}

_uint VIBufferTerrain::GetNumVerticesX() const
{
    return m_iNumVerticesX;
}

_uint VIBufferTerrain::GetNumVerticesZ() const
{
    return m_iNumVerticesZ;
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

HRESULT VIBufferTerrain::LoadHeightMap(const std::wstring& heightMapPath)
{
    _ulong dwByte = {};
    HANDLE hFile = CreateFile(heightMapPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    BITMAPFILEHEADER fh{};
    ReadFile(hFile, &fh, sizeof(fh), &dwByte, nullptr);

    BITMAPINFOHEADER ih{};
    ReadFile(hFile, &ih, sizeof(ih), &dwByte, nullptr);

    _uint* pPixels = new _uint[ih.biWidth * ih.biHeight];
    ReadFile(hFile, pPixels, sizeof(_uint) * ih.biWidth * ih.biHeight, &dwByte, nullptr);
    CloseHandle(hFile);

    m_iNumVerticesX = ih.biWidth;
    m_iNumVerticesZ = ih.biHeight;
    m_HeightMapData.assign(m_iNumVerticesZ, std::vector<float>(m_iNumVerticesX));

    for (int z = 0; z < ih.biHeight; ++z)
    {
        for (int x = 0; x < ih.biWidth; ++x)
        {
            _uint pixel = pPixels[z * ih.biWidth + x];
            float height = (pixel & 0x000000FF) / 10.0f;
            m_HeightMapData[z][x] = height;
        }
    }

    SafeDeleteArray(pPixels);
    return S_OK;

}
