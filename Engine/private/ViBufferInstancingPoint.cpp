#include "VIBufferInstancingPoint.h"

#include "EngineUtility.h"

VIBufferInstancingPoint::VIBufferInstancingPoint()
    : VIBufferInstancing{ }
{
}

VIBufferInstancingPoint::VIBufferInstancingPoint(const VIBufferInstancingPoint& Prototype)
    : VIBufferInstancing{ Prototype }
    , m_pInstanceVertices{ Prototype.m_pInstanceVertices }
    , m_pSpeeds{ Prototype.m_pSpeeds }
    , m_isLoop{ Prototype.m_isLoop }
    , m_vPivot{ Prototype.m_vPivot }
{
}

HRESULT VIBufferInstancingPoint::InitializePrototype(const INSTANCE_DESC* pDesc)
{
    const POINT_INSTANCE_DESC* pInstanceDesc = static_cast<const POINT_INSTANCE_DESC*>(pDesc);

    m_iNumVertexBuffers = 2;
    m_iNumVertices = 1;
    m_iVertexStride = sizeof(VTXPOS);

    m_iNumIndices = 0;
    m_iIndexStride = 0;
    m_eIndexFormat = DXGI_FORMAT_UNKNOWN;
    m_ePrimitive = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

    m_iNumInstance = pInstanceDesc->iNumInstance;
    m_iInstanceVertexStride = sizeof(VTXINSTANCEPARTICLE);
    m_iIndexCountPerInstance = 0;

#pragma region VERTEX_BUFFER
    D3D11_BUFFER_DESC           VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

    pVertices->vPosition = _float3(0.0f, 0.0f, 0.f);

    D3D11_SUBRESOURCE_DATA      VertexInitialData{};
    VertexInitialData.pSysMem = pVertices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&VBDesc, &VertexInitialData, &m_pVertexBuffer)))
        return E_FAIL;

    SafeDeleteArray(pVertices);

#pragma endregion

#pragma region INDEX_BUFFER 

#pragma endregion

#pragma region INSTANCE_BUFFER

    m_InstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
    m_InstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
    m_InstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    m_InstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    m_InstanceDesc.MiscFlags = 0;
    m_InstanceDesc.StructureByteStride = m_iInstanceVertexStride;

    m_pInstanceVertices = new VTXINSTANCEPARTICLE[m_iNumInstance];
    ZeroMemory(m_pInstanceVertices, sizeof(VTXINSTANCEPARTICLE) * m_iNumInstance);
    m_pSpeeds = new _float[m_iNumInstance];

    for (size_t i = 0; i < m_iNumInstance; i++)
    {
        _float      fScale = m_pEngineUtility->Random(pInstanceDesc->vScale.x, pInstanceDesc->vScale.y);

        m_pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
        m_pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
        m_pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
        m_pInstanceVertices[i].vTranslation = _float4(
            m_pEngineUtility->Random(pInstanceDesc->vCenter.x - pInstanceDesc->vRange.x * 0.5f, pInstanceDesc->vCenter.x + pInstanceDesc->vRange.x * 0.5f),
            m_pEngineUtility->Random(pInstanceDesc->vCenter.y - pInstanceDesc->vRange.y * 0.5f, pInstanceDesc->vCenter.y + pInstanceDesc->vRange.y * 0.5f),
            m_pEngineUtility->Random(pInstanceDesc->vCenter.z - pInstanceDesc->vRange.z * 0.5f, pInstanceDesc->vCenter.z + pInstanceDesc->vRange.z * 0.5f),
            1.f
        );
        m_pSpeeds[i] = m_pEngineUtility->Random(pInstanceDesc->vSpeed.x, pInstanceDesc->vSpeed.y);
        m_pInstanceVertices[i].vLifeTime = _float2(
            m_pEngineUtility->Random(pInstanceDesc->vLifeTime.x, pInstanceDesc->vLifeTime.y),
            0.0f
        );
    }

    m_vPivot = pInstanceDesc->vPivot;
    m_isLoop = pInstanceDesc->isLoop;

#pragma endregion


    return S_OK;
}

HRESULT VIBufferInstancingPoint::Initialize(void* pArg)
{
    D3D11_SUBRESOURCE_DATA      InstanceInitialData{};
    InstanceInitialData.pSysMem = m_pInstanceVertices;

    if (FAILED(m_pEngineUtility->GetDevice()->CreateBuffer(&m_InstanceDesc, &InstanceInitialData, &m_pVBInstance)))
        return E_FAIL;

    return S_OK;
}

HRESULT VIBufferInstancingPoint::BindBuffers()
{
    ID3D11Buffer* pVertexBuffers[] = {
        m_pVertexBuffer,
        m_pVBInstance,
    };

    _uint			iVertexStrides[] = {
        m_iVertexStride,
        m_iInstanceVertexStride,

    };

    _uint			iOffsets[] = {
        0,
        0
    };

    m_pEngineUtility->GetContext()->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
    m_pEngineUtility->GetContext()->IASetPrimitiveTopology(m_ePrimitive);

    return S_OK;
}

HRESULT VIBufferInstancingPoint::Render()
{
    m_pEngineUtility->GetContext()->DrawInstanced(1, m_iNumInstance, 0, 0);

    return S_OK;
}

void VIBufferInstancingPoint::Drop(_float fTimeDelta)
{
    D3D11_MAPPED_SUBRESOURCE        SubResource{};
    m_pEngineUtility->GetContext()->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

    VTXINSTANCEPARTICLE* pVertices = static_cast<VTXINSTANCEPARTICLE*>(SubResource.pData);

    for (size_t i = 0; i < m_iNumInstance; i++)
    {
        pVertices[i].vTranslation.y -= m_pSpeeds[i] * fTimeDelta;
        pVertices[i].vLifeTime.y += fTimeDelta;

        if (true == m_isLoop && pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
        {
            pVertices[i].vLifeTime.y = 0.f;
            pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
        }
    }

    m_pEngineUtility->GetContext()->Unmap(m_pVBInstance, 0);

}

void VIBufferInstancingPoint::Spread(_float fTimeDelta)
{
    D3D11_MAPPED_SUBRESOURCE        SubResource{};
    m_pEngineUtility->GetContext()->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

    VTXINSTANCEPARTICLE* pVertices = static_cast<VTXINSTANCEPARTICLE*>(SubResource.pData);

    for (size_t i = 0; i < m_iNumInstance; i++)
    {
        _vector     vMoveDir = XMVectorSetW(XMLoadFloat4(&pVertices[i].vTranslation) - XMLoadFloat3(&m_vPivot), 0.f);

        XMStoreFloat4(&pVertices[i].vTranslation,
            XMLoadFloat4(&pVertices[i].vTranslation) + XMVector3Normalize(vMoveDir) * m_pSpeeds[i] * fTimeDelta);

        pVertices[i].vLifeTime.y += fTimeDelta;

        if (true == m_isLoop && pVertices[i].vLifeTime.y >= pVertices[i].vLifeTime.x)
        {
            pVertices[i].vLifeTime.y = 0.f;
            pVertices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
        }
    }

    m_pEngineUtility->GetContext()->Unmap(m_pVBInstance, 0);
}

VIBufferInstancingPoint* VIBufferInstancingPoint::Create(const INSTANCE_DESC* pDesc)
{
    VIBufferInstancingPoint* pInstance = new VIBufferInstancingPoint();

    if (FAILED(pInstance->InitializePrototype(pDesc)))
    {
        MSG_BOX("Failed to Created : VIBufferInstancingPoint");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Component* VIBufferInstancingPoint::Clone(void* pArg)
{
    VIBufferInstancingPoint* pInstance = new VIBufferInstancingPoint(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : VIBufferInstancingPoint");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void VIBufferInstancingPoint::Free()
{
    __super::Free();

    if (false == m_isCloned)
    {
        SafeDeleteArray(m_pInstanceVertices);
        SafeDeleteArray(m_pSpeeds);
    }

}
