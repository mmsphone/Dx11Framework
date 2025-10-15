#include "Cell.h"
#include "VIBufferCell.h"

Cell::Cell()
{
}

_vector Cell::GetPoint(POINTTYPE ePoint) const
{
    return XMLoadFloat3(&m_vPoints[ePoint]);
}

_uint Cell::GetIndex() const
{
    return m_iIndex;
}

void Cell::SetNeighbor(LINETYPE eLine, Cell* pNeighbor)
{
    m_iNeighborIndices[eLine] = pNeighbor->m_iIndex;
}

HRESULT Cell::Initialize(const _float3* pPoints, _uint iIndex)
{
    memcpy(m_vPoints, pPoints, sizeof(_float3) * POINT_END);

    m_iIndex = iIndex;

    m_vNormals[LINETYPE::AB] = _float3((m_vPoints[POINTTYPE::B].z - m_vPoints[POINTTYPE::A].z) * -1.f, 0.f, m_vPoints[POINTTYPE::B].x - m_vPoints[POINTTYPE::A].x);
    m_vNormals[LINETYPE::BC] = _float3((m_vPoints[POINTTYPE::C].z - m_vPoints[POINTTYPE::B].z) * -1.f, 0.f, m_vPoints[POINTTYPE::C].x - m_vPoints[POINTTYPE::B].x);
    m_vNormals[LINETYPE::CA] = _float3((m_vPoints[POINTTYPE::A].z - m_vPoints[POINTTYPE::C].z) * -1.f, 0.f, m_vPoints[POINTTYPE::A].x - m_vPoints[POINTTYPE::C].x);

#ifdef _DEBUG
    m_pDebugBuffer = VIBufferCell::Create(m_vPoints);
    if (nullptr == m_pDebugBuffer)
        return E_FAIL;
#endif

    return S_OK;
}

_bool Cell::isIn(_fvector vResultPos, _int* pNeighborIndex)
{
    for (size_t i = 0; i < LINE_END; i++)
    {
        _vector vNormal = XMVector3Normalize(XMLoadFloat3(&m_vNormals[i]));
        _vector vDir = XMVector3Normalize(vResultPos - XMLoadFloat3(&m_vPoints[i]));

        if (0 < XMVectorGetX(XMVector3Dot(vNormal, vDir)))
        {
            *pNeighborIndex = m_iNeighborIndices[i];
            return false;
        }
    }
    return true;
}

_bool Cell::ComparePoints(_fvector vSour, _fvector vDest)
{
    if (true == XMVector3Equal(vSour, XMLoadFloat3(&m_vPoints[POINTTYPE::A])))
    {
        if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINTTYPE::B])))
            return true;
        if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINTTYPE::C])))
            return true;
    }

    if (true == XMVector3Equal(vSour, XMLoadFloat3(&m_vPoints[POINTTYPE::B])))
    {
        if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINTTYPE::C])))
            return true;
        if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINTTYPE::A])))
            return true;
    }

    if (true == XMVector3Equal(vSour, XMLoadFloat3(&m_vPoints[POINTTYPE::C])))
    {
        if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINTTYPE::A])))
            return true;
        if (true == XMVector3Equal(vDest, XMLoadFloat3(&m_vPoints[POINTTYPE::B])))
            return true;
    }

    return false;
}

_float Cell::ComputeHeight(_fvector vResultPos)
{
    _float4 vPlane = {};

    XMStoreFloat4(&vPlane, XMPlaneFromPoints(
        XMLoadFloat3(&m_vPoints[POINTTYPE::A]),
        XMLoadFloat3(&m_vPoints[POINTTYPE::B]),
        XMLoadFloat3(&m_vPoints[POINTTYPE::C])
    ));

    /*
    ax + by + cz + d = 0;
    y = (-ax - cz - d) / b;
    */

    float   fy = (-vPlane.x * XMVectorGetX(vResultPos) - vPlane.z * XMVectorGetZ(vResultPos) - vPlane.w) / vPlane.y;

    return fy;
}

#ifdef _DEBUG
HRESULT Cell::Render()
{
    m_pDebugBuffer->BindBuffers();
    m_pDebugBuffer->Render();

    return S_OK;
}
#endif

Cell* Cell::Create( const _float3* pPoints, _uint iIndex)
{
    Cell* pInstance = new Cell();

    if (FAILED(pInstance->Initialize(pPoints, iIndex)))
    {
        MSG_BOX("Failed to Created : CCell");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Cell::Free()
{
    __super::Free();

#ifdef _DEBUG
    SafeRelease(m_pDebugBuffer);
#endif
}
