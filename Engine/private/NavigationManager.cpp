#include "NavigationManager.h"

#include "EngineUtility.h"

#include "Cell.h"
#include "Shader.h"

NavigationManager::NavigationManager()
    :m_pEngineUtility{EngineUtility::GetInstance()}
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT NavigationManager::Initialize()
{
#ifdef _DEBUG
    m_pShader = Shader::Create( TEXT("../bin/Shader/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
    if (m_pShader == nullptr)
        return E_FAIL;
#endif
    return S_OK;
}

#ifdef _DEBUG
HRESULT NavigationManager::Render()
{
    if (FAILED(m_pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;

    _float4x4 WorldMatrix{};
    XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

    _float4     vColor = _float4(1.f, 1.f, 1.f, 1.f);
    m_pShader->BindRawValue("g_vColor", &vColor, sizeof(_float4));

    if (FAILED(m_pShader->BindMatrix("g_WorldMatrix", &WorldMatrix)))
        return E_FAIL;

    m_pShader->Begin(0);
    for (auto& pCell : m_Cells)
    {
        pCell->Render();
    }
    return S_OK;
}
#endif

void NavigationManager::AddTempPoint(const _float3& point)
{
    m_TempPoints.push_back(point);
    if (m_TempPoints.size() == 3)
    {
        // ⚠️ 외부 정렬/검사 제거: AddCell이 모두 처리
        _float3 tri[3] = { m_TempPoints[0], m_TempPoints[1], m_TempPoints[2] };
        AddCell(tri);
        m_TempPoints.clear();
    }
}
void NavigationManager::ClearTempPoints()
{
    m_TempPoints.clear();
}
void NavigationManager::RemoveRecentCell()
{
    if (m_Cells.empty()) 
        return;

    const _int removedIdx = static_cast<_int>(m_Cells.size() - 1);

    for (auto* c : m_Cells)
    {
        if (!c) continue;
        for (int e = 0; e < static_cast<int>(LINE_END); ++e)
        {
            if (c->GetNeighborIndex(static_cast<LINETYPE>(e)) == removedIdx)
                c->SetNeighborIndex(static_cast<LINETYPE>(e), -1);
        }
    }

    SafeRelease(m_Cells.back());
    m_Cells.pop_back();
}

void NavigationManager::ClearCells()
{
    if (m_Cells.empty()) 
        return;

    for (auto& cell : m_Cells)
        SafeRelease(cell);
    m_Cells.clear();
    m_TempPoints.clear();
    m_EditingCell = -1;
}
void NavigationManager::SaveCells(const _char* pNavigationDataFile)
{
    if (m_Cells.empty())
        return;

    HANDLE hFile = CreateFileA(
        pNavigationDataFile,
        GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
        return;

    _ulong dwByte = 0;

    // [헤더] 셀 개수
    uint32_t count = static_cast<uint32_t>(m_Cells.size());
    WriteFile(hFile, &count, sizeof(uint32_t), &dwByte, nullptr);

    // [바디] 각 삼각형 3점
    for (auto& pCell : m_Cells)
    {
        _float3 vPoints[POINT_END];
        XMStoreFloat3(&vPoints[0], pCell->GetPoint(POINTTYPE::A));
        XMStoreFloat3(&vPoints[1], pCell->GetPoint(POINTTYPE::B));
        XMStoreFloat3(&vPoints[2], pCell->GetPoint(POINTTYPE::C));

        WriteFile(hFile, vPoints, sizeof(_float3) * POINT_END, &dwByte, nullptr);
    }

    CloseHandle(hFile);
}
void NavigationManager::LoadCells(const _char* pNavigationDataFile)
{
    HANDLE hFile = CreateFileA(pNavigationDataFile, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    // 파일 크기로 새/구 포맷 판별
    LARGE_INTEGER fsize{};
    GetFileSizeEx(hFile, &fsize);

    // 새 포맷: [u32 count] + count * (3 * float3)
    // 구 포맷: 반복적으로 3 * float3
    const DWORD triBytes = sizeof(_float3) * POINT_END;

    _ulong dwByte = 0;
    ClearCells();

    if (fsize.QuadPart >= (LONGLONG)sizeof(uint32_t))
    {
        // 먼저 count를 읽어보고, 남은 크기가 count * triBytes와 맞으면 새 포맷
        uint32_t count = 0;
        ReadFile(hFile, &count, sizeof(uint32_t), &dwByte, nullptr);
        LONGLONG remain = fsize.QuadPart - sizeof(uint32_t);

        if (remain == (LONGLONG)count * triBytes)
        {
            // 새 포맷
            for (uint32_t i = 0; i < count; ++i)
            {
                _float3 vPoints[POINT_END];
                ReadFile(hFile, vPoints, triBytes, &dwByte, nullptr);
                AddCell(vPoints);
            }
            CloseHandle(hFile);
            return;
        }
        else
        {
            // 구 포맷일 가능성 → 파일 포인터를 처음으로 되돌린 뒤 구 포맷 방식으로 읽기
            SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
        }
    }

    // 구 포맷: EOF까지 3점씩 읽기
    while (true)
    {
        _float3 vPoints[POINT_END] = {};
        ReadFile(hFile, vPoints, triBytes, &dwByte, nullptr);
        if (dwByte != triBytes)
            break;
        AddCell(vPoints);
    }

    CloseHandle(hFile);
}

_bool NavigationManager::IsInCell(_fvector vWorldPos, _int* pOutCellIndex)
{
    if (m_Cells.empty())
        return false;

    for (size_t i = 0; i < m_Cells.size(); ++i)
    {
        _int iNeighbor = -1;
        if (m_Cells[i]->isIn(vWorldPos, &iNeighbor))
        {
            if (pOutCellIndex)
                *pOutCellIndex = static_cast<_int>(i);
            return true;
        }
    }
    return false;
}

_bool NavigationManager::SetHeightOnCell(_fvector vWorldPos, _vector* pOutAdjustedPos)
{
    if (m_Cells.empty())
        return false;

    _int iCellIndex = -1;
    if (IsInCell(vWorldPos, &iCellIndex) == false)
        return false;

    float fHeight = m_Cells[iCellIndex]->ComputeHeight(vWorldPos);
    _vector vAdjusted = XMVectorSetY(vWorldPos, fHeight);

    if (pOutAdjustedPos)
        *pOutAdjustedPos = vAdjusted;
    return true;
}

const vector<class Cell*>& NavigationManager::GetCells() const
{
    return m_Cells;
}

_bool NavigationManager::Edit_AddTriangleOnEdge(_int cellId, _fvector pickedPoint, _float weldEps)
{
    if (cellId < 0 || m_Cells.empty() || static_cast<size_t>(cellId) >= m_Cells.size())
        return false;

    // 기존 셀의 3점(A,B,C)
    _float3 cellPoints[3] = {};
    for (int i = 0; i < 3; ++i)
        XMStoreFloat3(&cellPoints[i], m_Cells[cellId]->GetPoint(static_cast<POINTTYPE>(i)));

    // 피킹한 점
    _float3 pickedWS{};
    XMStoreFloat3(&pickedWS, pickedPoint);

    // 1) 피킹점에서 가장 가까운 에지 찾기
    float    nearDistance = FLT_MAX;
    LINETYPE nearestEdge = LINETYPE::AB;
    for (int line = 0; line < (int)LINE_END; ++line)
    {
        _float3 pointA, pointB;
        EdgeEndpoints(cellPoints, static_cast<LINETYPE>(line), pointA, pointB);
        _float distance = DistPointToSegmentXZ(pickedWS, pointA, pointB);
        if (distance < nearDistance) { nearDistance = distance; nearestEdge = static_cast<LINETYPE>(line); }
    }

    // 2) 베이스 에지의 두 점 + 피킹 점 => 새 삼각형 1개 (추가용)
    _float3 pointA, pointB;
    EdgeEndpoints(cellPoints, nearestEdge, pointA, pointB);

    // 용접(피킹점이 꼭짓점에 매우 가까우면 스냅)
    SnapIfNear(pickedWS, pointA, weldEps);
    SnapIfNear(pickedWS, pointB, weldEps);

    _float3 newCellPoints[3] = { pointA, pointB, pickedWS };

    // 3) 새 셀 추가
    AddCell(newCellPoints);     // 내부에서 인덱스 부여 + 디버그 버퍼 준비

    return true;
}

_bool NavigationManager::Edit_AddTriangleAtSharedVertex(_int cellA, _int cellB, _float weldEps)
{
    if (m_Cells.empty()) return false;
    if (cellA < 0 || cellB < 0) return false;
    if (static_cast<size_t>(cellA) >= m_Cells.size()) return false;
    if (static_cast<size_t>(cellB) >= m_Cells.size()) return false;
    if (cellA == cellB) return false;

    // A, B 삼각형 좌표 가져오기
    _float3 A[3], B[3];
    for (int i = 0; i < 3; ++i) {
        XMStoreFloat3(&A[i], m_Cells[cellA]->GetPoint(static_cast<POINTTYPE>(i)));
        XMStoreFloat3(&B[i], m_Cells[cellB]->GetPoint(static_cast<POINTTYPE>(i)));
    }

    const float eps2 = weldEps * weldEps;
    auto almostEq = [&](const _float3& p, const _float3& q) -> bool {
        float dx = p.x - q.x, dy = p.y - q.y, dz = p.z - q.z;
        return (dx * dx + dy * dy + dz * dz) <= eps2;
        };
    auto distXZ2 = [&](const _float3& p, const _float3& q) -> float {
        float dx = p.x - q.x, dz = p.z - q.z;
        return dx * dx + dz * dz;
        };

    // 1) 공유 꼭짓점 찾기 (정확히 1개여야 함)
    int sharedAi = -1, sharedBi = -1;
    int sharedCount = 0;
    for (int ia = 0; ia < 3; ++ia) {
        for (int ib = 0; ib < 3; ++ib) {
            if (almostEq(A[ia], B[ib])) {
                sharedAi = ia; sharedBi = ib;
                ++sharedCount;
            }
        }
    }
    if (sharedCount != 1)
        return false; // 한 점만 공유하는 케이스가 아니면 실패

    _float3 P = A[sharedAi]; // 공유점(=B[sharedBi])

    // 2) 비공유 꼭짓점 후보들 모으기
    _float3 Aopts[2], Bopts[2];
    int ia2 = 0, ib2 = 0;
    for (int i = 0; i < 3; ++i) if (i != sharedAi) Aopts[ia2++] = A[i];
    for (int i = 0; i < 3; ++i) if (i != sharedBi) Bopts[ib2++] = B[i];

    // 3) 두 집합에서 XZ 거리가 가장 가까운 페어 선택
    int bestA = 0, bestB = 0;
    float bestD2 = FLT_MAX;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            float d2 = distXZ2(Aopts[i], Bopts[j]);
            if (d2 < bestD2) { bestD2 = d2; bestA = i; bestB = j; }
        }
    }

    // 4) 새 삼각형 구성: (공유점 P, A측 선택점, B측 선택점)
    _float3 tri[3] = { P, Aopts[bestA], Bopts[bestB] };

    // 용접: 선택점들이 공유점에 매우 가깝다면 스냅
    SnapIfNear(tri[1], tri[0], weldEps);
    SnapIfNear(tri[2], tri[0], weldEps);

    // 5) 추가 (면적/중복/정렬/이웃은 AddCell 내부가 처리)
    AddCell(tri);
    return true;
}

_bool NavigationManager::RandomPointAround(_fvector center, _float radius, _float3* outPos, _uint maxTrials)
{
    if (m_Cells.empty() || !outPos) return false;

    for (_uint i = 0; i < maxTrials; ++i)
    {
        float ox = m_pEngineUtility->Random(-radius, radius);
        float oz = m_pEngineUtility->Random(-radius, radius);
        if (ox * ox + oz * oz > radius * radius)
            continue;

        _vector c = center;
        _vector test = XMVectorSet(XMVectorGetX(c) + ox, XMVectorGetY(c), XMVectorGetZ(c) + oz, 1.f);

        _int cellId = -1;
        if (!IsInCell(test, &cellId))     
            continue;

        // 셀 평면 높이에 맞춰서 y 보정
        float y = m_Cells[cellId]->ComputeHeight(test);
        XMStoreFloat3(outPos, XMVectorSet(XMVectorGetX(test), y, XMVectorGetZ(test), 1.f));
        return true;
    }
    return false;
}

NavigationManager* NavigationManager::Create()
{
    NavigationManager* pInstance = new NavigationManager();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : NavigationManager");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void NavigationManager::Free()
{
    __super::Free();

    SafeRelease(m_pEngineUtility);
    for (auto& pCell : m_Cells)
        SafeRelease(pCell);
    m_Cells.clear();
    
#ifdef _DEBUG
    SafeRelease(m_pShader);
#endif
}

void NavigationManager::AddCell(_float3* pPoints)
{
    if (pPoints == nullptr) 
        return;

    // 시계방향 보정 + 퇴화 검사
    auto signedAreaXZ = [](const _float3& a, const _float3& b, const _float3& c) {
        return 0.5f * ((b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x));
        };
    std::vector<_float3> tri = { pPoints[0], pPoints[1], pPoints[2] };

    // 면적 체크
    if (fabsf(signedAreaXZ(tri[0], tri[1], tri[2])) < 1e-6f)
        return;

    // CW 정렬
    SortPointsClockWise(tri);

    // 중복 검사(기존 셀과 동일 정점 셋이면 스킵)
    const float eps = 1e-3f;
    auto almostEq = [&](const _float3& a, const _float3& b) {
        float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return (dx * dx + dy * dy + dz * dz) <= (eps * eps);
        };
    auto sameTri = [&](const _float3 t[3], const _float3 s[3])->bool {
        // 순서 무시, 집합 비교
        int match = 0;
        for (int i = 0; i < 3; ++i) {
            bool found = false;
            for (int j = 0; j < 3; ++j) {
                if (almostEq(t[i], s[j])) { found = true; break; }
            }
            if (found) ++match; else return false;
        }
        return match == 3;
        };

    _float3 newTri[3] = { tri[0], tri[1], tri[2] };
    for (auto* c : m_Cells) {
        _float3 a, b, cpt;
        XMStoreFloat3(&a, c->GetPoint(POINTTYPE::A));
        XMStoreFloat3(&b, c->GetPoint(POINTTYPE::B));
        XMStoreFloat3(&cpt, c->GetPoint(POINTTYPE::C));
        _float3 oldTri[3] = { a, b, cpt };
        if (sameTri(newTri, oldTri))
            return;
    }

    _uint iCellIndex = (m_Cells.empty()) ? 0 : (m_Cells.back()->GetIndex() + 1);
    Cell* pCell = Cell::Create(newTri, iCellIndex);
    if (!pCell) return;

    pCell->SetNeighborIndex(LINETYPE::AB, -1);
    pCell->SetNeighborIndex(LINETYPE::BC, -1);
    pCell->SetNeighborIndex(LINETYPE::CA, -1);

    m_Cells.push_back(pCell);
    SetNeighborsForNewCell(iCellIndex);
}
void NavigationManager::SetNeighborsForNewCell(_int newCellIndex)
{
    if (newCellIndex < 0 || static_cast<size_t>(newCellIndex) >= m_Cells.size()) return;

    const float eps = 1e-3f;
    Cell* n = m_Cells[newCellIndex];

    const _vector nA = n->GetPoint(POINTTYPE::A);
    const _vector nB = n->GetPoint(POINTTYPE::B);
    const _vector nC = n->GetPoint(POINTTYPE::C);

    struct Edge { LINETYPE e; _vector u, v; } edges[3] = {
        { LINETYPE::AB, nA, nB },
        { LINETYPE::BC, nB, nC },
        { LINETYPE::CA, nC, nA }
    };

    auto matchEdge = [&](Cell* cell, const _vector& s, const _vector& t, LINETYPE& outEdge)->bool {
        if (!cell->ComparePointsEps(s, t, eps)) return false;

        const _vector A = cell->GetPoint(POINTTYPE::A);
        const _vector B = cell->GetPoint(POINTTYPE::B);
        const _vector C = cell->GetPoint(POINTTYPE::C);
        auto eq = [&](const _vector& x, const _vector& y) {
            _float3 xf, yf; XMStoreFloat3(&xf, x); XMStoreFloat3(&yf, y);
            const float dx = xf.x - yf.x, dy = xf.y - yf.y, dz = xf.z - yf.z;
            return (dx * dx + dy * dy + dz * dz) <= eps * eps;
            };
        if ((eq(s, A) && eq(t, B)) || (eq(s, B) && eq(t, A))) { outEdge = LINETYPE::AB; return true; }
        if ((eq(s, B) && eq(t, C)) || (eq(s, C) && eq(t, B))) { outEdge = LINETYPE::BC; return true; }
        if ((eq(s, C) && eq(t, A)) || (eq(s, A) && eq(t, C))) { outEdge = LINETYPE::CA; return true; }
        return false;
        };

    for (size_t j = 0; j < m_Cells.size(); ++j)
    {
        if (static_cast<_int>(j) == newCellIndex) continue;
        Cell* d = m_Cells[j];

        for (auto& e : edges)
        {
            if (!d->ComparePointsEps(e.u, e.v, eps)) continue;

            // 새 셀 쪽 이웃 인덱스 설정
            n->SetNeighborIndex(e.e, static_cast<_int>(j));

            // 상대 셀의 동일 에지를 찾아 역방향 이웃 인덱스도 설정
            LINETYPE dEdge{};
            if (matchEdge(d, e.u, e.v, dEdge))
                d->SetNeighborIndex(dEdge, newCellIndex);
        }
    }
}

void NavigationManager::SortPointsClockWise(std::vector<_float3>& pts)
{
    if (pts.size() != 3) return;

    auto signedAreaXZ = [](const _float3& a, const _float3& b, const _float3& c) {
        return 0.5f * ((b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x));
        };

    // CCW(양수)면 B<->C 스왑해서 CW로 만든다 (Y+ 기준)
    float area = signedAreaXZ(pts[0], pts[1], pts[2]);
    if (area > 0.0f) {
        std::swap(pts[1], pts[2]);
    }
}

void NavigationManager::EdgeEndpoints(const _float3 tri[3], LINETYPE e, _float3& outA, _float3& outB) const
{
    switch (e)
    {
    case LINETYPE::AB: outA = tri[POINTTYPE::A]; outB = tri[POINTTYPE::B]; break;
    case LINETYPE::BC: outA = tri[POINTTYPE::B]; outB = tri[POINTTYPE::C]; break;
    case LINETYPE::CA: outA = tri[POINTTYPE::C]; outB = tri[POINTTYPE::A]; break;
    default:           outA = tri[POINTTYPE::A]; outB = tri[POINTTYPE::B]; break;
    }
}

float NavigationManager::DistPointToSegmentXZ(const _float3& p, const _float3& a, const _float3& b) const
{
    const float ax = a.x, az = a.z;
    const float bx = b.x, bz = b.z;
    const float px = p.x, pz = p.z;

    const float vx = bx - ax;
    const float vz = bz - az;

    const float wx = px - ax;
    const float wz = pz - az;

    const float vv = vx * vx + vz * vz;
    if (vv <= 1e-12f)
    {
        // A와 B가 거의 같은 점
        const float dx = px - ax;
        const float dz = pz - az;
        return sqrtf(dx * dx + dz * dz);
    }

    float t = (wx * vx + wz * vz) / vv;   // 구간 투영 계수
    if (t < 0.f) t = 0.f;
    else if (t > 1.f) t = 1.f;

    const float qx = ax + t * vx;
    const float qz = az + t * vz;

    const float dx = px - qx;
    const float dz = pz - qz;
    return sqrtf(dx * dx + dz * dz);
}

void NavigationManager::SnapIfNear(_float3& inoutP, const _float3& target, float eps) const
{
    const float dx = inoutP.x - target.x;
    const float dy = inoutP.y - target.y;
    const float dz = inoutP.z - target.z;
    if ((dx * dx + dy * dy + dz * dz) <= eps * eps)
        inoutP = target;
}
