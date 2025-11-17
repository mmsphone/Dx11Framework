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

_uint Cell::GetNeighborIndex(LINETYPE eLine)
{
    return m_iNeighborIndices[eLine];
}

void Cell::SetNeighborIndex(LINETYPE eLine, _uint iNeighborIndex)
{
    if (iNeighborIndex == -1) {
        m_iNeighborIndices[eLine] = -1;
        return;
    }

    m_iNeighborIndices[eLine] = iNeighborIndex;
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
    const POINTTYPE anchor[LINE_END] = { POINTTYPE::A, POINTTYPE::B, POINTTYPE::C };

    for (size_t i = 0; i < LINE_END; i++)
    {
        _vector vNormal = XMVector3Normalize(XMLoadFloat3(&m_vNormals[i]));
        _vector vDir = XMVector3Normalize(vResultPos - XMLoadFloat3(&m_vPoints[anchor[i]]));

        if (XMVectorGetX(XMVector3Dot(vNormal, vDir)) > 0.f)
        {
            if (pNeighborIndex) *pNeighborIndex = m_iNeighborIndices[i];
            return false; // 밖
        }
    }
    return true; // 안
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

_bool Cell::ComparePointsEps(_fvector vSour, _fvector vDest, _float eps)
{
    _float3 s, d;
    XMStoreFloat3(&s, vSour);
    XMStoreFloat3(&d, vDest);

    const _float3 pa = m_vPoints[POINTTYPE::A];
    const _float3 pb = m_vPoints[POINTTYPE::B];
    const _float3 pc = m_vPoints[POINTTYPE::C];

    auto sameEdge = [&](const _float3& e0, const _float3& e1)->_bool {
        // (s,d) == (e0,e1) or (e1,e0)
        return (AlmostEqual3D(s, e0, eps) && AlmostEqual3D(d, e1, eps)) ||
            (AlmostEqual3D(s, e1, eps) && AlmostEqual3D(d, e0, eps));
        };

    if (sameEdge(pa, pb)) return true;
    if (sameEdge(pb, pc)) return true;
    if (sameEdge(pc, pa)) return true;
    return false;
}

_float Cell::ComputeHeight(_fvector vResultPos)
{
    _float4 pl{};
    XMStoreFloat4(&pl, XMPlaneFromPoints(
        XMLoadFloat3(&m_vPoints[POINTTYPE::A]),
        XMLoadFloat3(&m_vPoints[POINTTYPE::B]),
        XMLoadFloat3(&m_vPoints[POINTTYPE::C])));

    const float EPS = 1e-6f;

    // 일반 케이스: y = (-ax - cz - d) / b
    if (fabsf(pl.y) > EPS)
    {
        return (-pl.x * XMVectorGetX(vResultPos)
            - pl.z * XMVectorGetZ(vResultPos)
            - pl.w) / pl.y;
    }

    // 특수 케이스: b ~= 0 → y고정 평면이 아니거나 수직에 가까움.
    // 평면과 수직(0,1,0) 광선 교차 이용:
    // 점 P0 = (x, any, z); 평면 n·X + d = 0
    // y를 직접 풀 수 없으니, (x,z)는 유지, y는 A,B,C의 평균 높이로 fallback
    // (실제론 평면 방정식과 최소제곱 투영 등을 쓰지만 여기선 안전한 근사)
    return (m_vPoints[0].y + m_vPoints[1].y + m_vPoints[2].y) / 3.f;
}

void Cell::SetIndex(_int iIndex)
{
    m_iIndex = iIndex;
}


#ifdef _DEBUG
HRESULT Cell::Render()
{
    m_pDebugBuffer->BindBuffers();
    m_pDebugBuffer->Render();

    return S_OK;
}
#endif

_bool Cell::AlmostEqual3D(const _float3& a, const _float3& b, float eps)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return (dx * dx + dy * dy + dz * dz) <= (eps * eps);
}

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
