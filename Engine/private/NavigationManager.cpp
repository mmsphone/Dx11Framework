#include "NavigationManager.h"

#include "EngineUtility.h"

#include "Cell.h"
#include "Shader.h"

inline _float3 ToFloat3(_fvector v)
{
    _float3 r;
    XMStoreFloat3(&r, v);
    return r;
}

inline float SquaredDist3(const _float3& a, const _float3& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

inline bool NearlyEqual3(const _float3& a, const _float3& b, float eps = 1e-4f)
{
    return SquaredDist3(a, b) <= eps * eps;
}

// XZ 평면 기준 signed area * 2
inline float TriArea2_XZ(const _float3& a, const _float3& b, const _float3& c)
{
    float abx = b.x - a.x;
    float abz = b.z - a.z;
    float acx = c.x - a.x;
    float acz = c.z - a.z;
    return abx * acz - abz * acx; // >0 이면 c가 ab의 왼쪽
}

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
    for (auto pCell : m_Cells)
    {
        if(pCell != nullptr)
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
void NavigationManager::RemoveCell(_int cellIndex)
{
    if (cellIndex < 0 || static_cast<size_t>(cellIndex) >= m_Cells.size())
        return;

    // -------------------------------------------
    // 1) 삭제 대상 셀 메모리 해제
    // -------------------------------------------
    SafeRelease(m_Cells[cellIndex]);

    // 벡터에서 제거
    m_Cells.erase(m_Cells.begin() + cellIndex);

    // -------------------------------------------
    // 2) 모든 셀의 이웃 index 재정리
    //    - cellIndex 보다 큰 index들은 1 감소
    //    - cellIndex 자체를 이웃으로 참조하던 경우 -1로 처리
    // -------------------------------------------

    for (auto* c : m_Cells)
    {
        for (int e = 0; e < (int)LINE_END; ++e)
        {
            int n = c->GetNeighborIndex(static_cast<LINETYPE>(e));
            if (n == cellIndex)
            {
                // 삭제 셀을 이웃으로 갖고 있었음
                c->SetNeighborIndex(static_cast<LINETYPE>(e), -1);
            }
            if (n > cellIndex)
            {
                // 삭제로 인해 index 1 감소시키기
                c->SetNeighborIndex(static_cast<LINETYPE>(e), n - 1);
            }
        }
    }

    // -------------------------------------------
    // 3) 남은 셀들의 Index 값을 0부터 재부여
    //    (Cell 내부에 인덱스를 가지고 있으므로 동기화 필요)
    // -------------------------------------------
    for (int i = 0; i < (int)m_Cells.size(); ++i)
    {
        m_Cells[i]->SetIndex(i);
    }

    // -------------------------------------------
    // 4) 옵션: 이웃 정보 전체 재계산
    // 정확도를 높이려면 아래처럼 전체 이웃을 다시 잡아도 됨
    // -------------------------------------------
    for (int i = 0; i < (int)m_Cells.size(); ++i)
    {
        // 기존 이웃 정보 초기화
        m_Cells[i]->SetNeighborIndex(LINETYPE::AB, -1);
        m_Cells[i]->SetNeighborIndex(LINETYPE::BC, -1);
        m_Cells[i]->SetNeighborIndex(LINETYPE::CA, -1);
    }

    // 모든 셀에 대해 다시 neighbor 연결
    for (int i = 0; i < (int)m_Cells.size(); ++i)
        SetNeighborsForNewCell(i);
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
void NavigationManager::LoadCells(const _char* pNavigationDataFilePath)
{
    HANDLE hFile = CreateFileA(pNavigationDataFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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

_float NavigationManager::GetHeightPosOnCell(_vector* pPos, const _int& pCellIndex)
{
    return m_Cells[pCellIndex]->ComputeHeight(*pPos);
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

_bool NavigationManager::GetSlideVectorOnCell(_fvector pos, _fvector delta, _int cellIndex, _vector* outSlideVector) const
{
    if (!outSlideVector) return false;
    if (cellIndex < 0 || static_cast<size_t>(cellIndex) >= m_Cells.size()) return false;

    // 현재/목표
    _float3 p; XMStoreFloat3(&p, pos);
    _float3 d; XMStoreFloat3(&d, delta);

    // 삼각형 정점 (CW 정렬 가정: AddCell에서 보정)
    _float3 T[3];
    XMStoreFloat3(&T[0], m_Cells[cellIndex]->GetPoint(POINTTYPE::A));
    XMStoreFloat3(&T[1], m_Cells[cellIndex]->GetPoint(POINTTYPE::B));
    XMStoreFloat3(&T[2], m_Cells[cellIndex]->GetPoint(POINTTYPE::C));

    auto cross2D = [](_float3 a, _float3 b)->float { return a.x * b.z - a.z * b.x; };
    auto sub = [](_float3 a, _float3 b)->_float3 { return _float3{ a.x - b.x, 0.f, a.z - b.z }; };
    auto dot2D = [](_float3 a, _float3 b)->float { return a.x * b.x + a.z * b.z; };
    auto len2D = [](_float3 a)->float { return sqrtf(a.x * a.x + a.z * a.z); };
    auto norm2D = [&](_float3 v)->_float3 {
        float L = len2D(v); if (L <= 1e-8f) return _float3{ 0,0,0 };
        return _float3{ v.x / L, 0.f, v.z / L };
        };

    // 목표 위치(이동 적용)
    _float3 q = _float3{ p.x + d.x, p.y + d.y, p.z + d.z };

    // 삼각형 내부 판정(2D XZ). CW 삼각형의 내부는 각 에지에 대해 sign <= 0 이어야 함
    auto signedSide = [&](const _float3& A, const _float3& B, const _float3& P)->float {
        _float3 AB = sub(B, A);
        _float3 AP = sub(P, A);
        return cross2D(AB, AP); // CW 내부면 보통 <= 0
        };

    float sAB = signedSide(T[0], T[1], q);
    float sBC = signedSide(T[1], T[2], q);
    float sCA = signedSide(T[2], T[0], q);

    // 모두 <= 0 이면 안 미끄러져도 됨(그냥 원하는 이동)
    if (sAB <= 0.f && sBC <= 0.f && sCA <= 0.f) {
        *outSlideVector = delta;
        return true;
    }

    // 밖이라면 "가장 크게 양수인"(= 가장 많이 위반한) 에지를 고른다
    struct Hit { LINETYPE e; float s; _float3 A, B; };
    Hit cand[3] = {
        { LINETYPE::AB, sAB, T[0], T[1] },
        { LINETYPE::BC, sBC, T[1], T[2] },
        { LINETYPE::CA, sCA, T[2], T[0] },
    };
    float bestS = -FLT_MAX; int bestI = -1;
    for (int i = 0; i < 3; ++i) {
        if (cand[i].s > bestS) { bestS = cand[i].s; bestI = i; }
    }
    if (bestI < 0) return false; // 이론상 도달 X

    // 선택 에지
    _float3 EA = cand[bestI].A;
    _float3 EB = cand[bestI].B;

    // 에지 방향 단위 벡터(XZ)
    _float3 edgeDir = norm2D(sub(EB, EA));
    if (len2D(edgeDir) <= 1e-7f) {
        // 퇴화 에지: 이동을 0으로
        *outSlideVector = XMVectorZero();
        return true;
    }

    // 원 이동 델타의 XZ를 에지 방향으로 정사영 → 슬라이드 벡터(XZ)
    _float3 dXZ = _float3{ d.x, 0.f, d.z };
    float proj = dot2D(dXZ, edgeDir);
    _float3 slideXZ = _float3{ edgeDir.x * proj, 0.f, edgeDir.z * proj };

    // 최종 슬라이드 목표 위치
    _float3 qSlide = _float3{ p.x + slideXZ.x, p.y + d.y, p.z + slideXZ.z };

    // 여전히 밖이면(코너 등) 에지 위의 최근접점으로 클램프 후, 안쪽으로 ε 밀어넣기
    float sABs = signedSide(T[0], T[1], qSlide);
    float sBCs = signedSide(T[1], T[2], qSlide);
    float sCAs = signedSide(T[2], T[0], qSlide);

    if (!(sABs <= 0.f && sBCs <= 0.f && sCAs <= 0.f))
    {
        // 에지 선분의 최근접점
        _float3 AE = sub(qSlide, EA);
        _float3 E = sub(EB, EA);
        float Elen2 = dot2D(E, E);
        float t = (Elen2 > 1e-8f) ? (dot2D(AE, E) / Elen2) : 0.f;
        if (t < 0.f) t = 0.f; else if (t > 1.f) t = 1.f;

        _float3 onEdge = _float3{ EA.x + E.x * t, p.y + d.y, EA.z + E.z * t };

        // 에지의 안쪽 법선( CW 기준 내부는 cross(Edge, inward) 부호가 음수 )
        // 에지 법선(안쪽) = 에지 방향을 오른쪽으로 90도 회전한 벡터
        _float3 inwardN = _float3{ -edgeDir.z, 0.f, edgeDir.x }; // CW에서 대체로 내부로 향함
        const float pushEps = 1e-3f;

        _float3 pushed = _float3{ onEdge.x + inwardN.x * pushEps,
                                  onEdge.y,
                                  onEdge.z + inwardN.z * pushEps };

        slideXZ = _float3{ pushed.x - p.x, 0.f, pushed.z - p.z };
    }

    // out: Y는 원래 delta.y 유지(보통 0). XZ는 slide로 교체
    _vector out = XMVectorSet(slideXZ.x, d.y, slideXZ.z, 0.f);
    *outSlideVector = out;
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

    if (sharedCount == 1)
    {
        _float3 P = A[sharedAi];

        _float3 Aopts[2], Bopts[2];
        int ia2 = 0, ib2 = 0;
        for (int i = 0; i < 3; ++i) if (i != sharedAi) Aopts[ia2++] = A[i];
        for (int i = 0; i < 3; ++i) if (i != sharedBi) Bopts[ib2++] = B[i];

        int bestA = 0, bestB = 0;
        float bestD2 = FLT_MAX;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                float d2 = distXZ2(Aopts[i], Bopts[j]);
                if (d2 < bestD2) { bestD2 = d2; bestA = i; bestB = j; }
            }
        }

        _float3 tri[3] = { P, Aopts[bestA], Bopts[bestB] };
        SnapIfNear(tri[1], tri[0], weldEps);
        SnapIfNear(tri[2], tri[0], weldEps);
        AddCell(tri);
        return true;
    }

    if (sharedCount == 0)
    {
        LINETYPE eA{}, eB{};
        _float3 A0, A1, B0, B1;
        if (!FindNearestEdgePairXZ(A, B, eA, eB, A0, A1, B0, B1))
            return false;

        // 용접: 각각 대응되는 쌍을 스냅
        SnapIfNear(B0, A0, weldEps);
        SnapIfNear(B1, A1, weldEps);

        // 사각형(A0-A1-B1-B0)을 두 개의 삼각형으로 분해
        _float3 tri1[3] = { A0, A1, B0 }; // 대각선 A1-B0
        _float3 tri2[3] = { B0, A1, B1 };

        // 퇴화 방지(혹시 스냅 때문에 면적 0이 되면 대각선 반대로 한번 더 시도)
        auto areaXZ2 = [](_float3 p, _float3 q, _float3 r)->float {
            float s = ((q.x - p.x) * (r.z - p.z) - (q.z - p.z) * (r.x - p.x));
            return s * s;
            };
        const float minArea2 = 1e-12f;

        bool ok1 = areaXZ2(tri1[0], tri1[1], tri1[2]) > minArea2;
        bool ok2 = areaXZ2(tri2[0], tri2[1], tri2[2]) > minArea2;

        if (!ok1 || !ok2)
        {
            // 대각선을 A0-B1로 바꾼 분해 시도
            _float3 t1[3] = { A0, B1, B0 };
            _float3 t2[3] = { A0, A1, B1 };
            bool ok1b = areaXZ2(t1[0], t1[1], t1[2]) > minArea2;
            bool ok2b = areaXZ2(t2[0], t2[1], t2[2]) > minArea2;
            if (ok1b && ok2b) { memcpy(tri1, t1, sizeof(t1)); memcpy(tri2, t2, sizeof(t2)); ok1 = ok2 = true; }
        }

        if (!ok1 || !ok2) return false;

        AddCell(tri1);
        AddCell(tri2);
        return true;
    }
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


_bool NavigationManager::FindPath(_fvector startPos, _fvector goalPos, vector<_int>& outCellPath)
{
    outCellPath.clear();

    if (m_Cells.empty())
        return false;

    _int startCell = {};
    IsInCell(startPos, &startCell); // 네가 쓰는 함수 이름에 맞게
    _int goalCell = {};
    IsInCell(goalPos, &goalCell);

    if (startCell < 0 || goalCell < 0)
        return false;

    if (startCell == goalCell)
    {
        outCellPath.push_back(startCell);
        return true;
    }

    const size_t cellCount = m_Cells.size();

    // 2) A* 노드 배열 준비
    std::vector<AStarNode> nodes(cellCount);

    // 휴리스틱: 셀 중심 간 거리
    auto Heuristic = [&](int cellIdx) -> float
        {
            _float3 c = GetCellCenter(cellIdx);
            _float3 gg;
            XMStoreFloat3(&gg, goalPos);

            _float3 d{
                gg.x - c.x,
                gg.y - c.y,
                gg.z - c.z
            };

            return sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
        };

    // 셀 중심 캐시 (약간의 최적화용, 원하면 생략해도 됨)
    std::vector<_float3> centers(cellCount);
    for (size_t i = 0; i < cellCount; ++i)
        centers[i] = GetCellCenter((_int)i);

    auto DistanceCells = [&](int a, int b) -> float
        {
            const _float3& ca = centers[a];
            const _float3& cb = centers[b];

            _float3 d{
                cb.x - ca.x,
                cb.y - ca.y,
                cb.z - ca.z
            };
            return sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
        };

    // 3) 시작 노드 세팅
    nodes[startCell].g = 0.f;
    nodes[startCell].f = Heuristic(startCell);

    priority_queue<OpenEntry, vector<OpenEntry>, CompareOpenEntry> open;
    open.push(OpenEntry{ nodes[startCell].f, startCell });

    std::vector<_int> neighbors;

    // 4) 메인 루프
    while (!open.empty())
    {
        _int current = open.top().index;
        open.pop();

        // 이미 처리 끝난 노드면 스킵 (중복 push 방지용)
        if (nodes[current].closed)
            continue;

        // 목표 셀 도달
        if (current == goalCell)
        {
            // 5) 경로 재구성
            _int c = goalCell;
            while (c != -1)
            {
                outCellPath.push_back(c);
                c = nodes[c].parent;
            }
            std::reverse(outCellPath.begin(), outCellPath.end());
            return true;
        }

        nodes[current].closed = true;

        // 이웃 순회
        GetNeighborIndices(current, neighbors);

        for (_int nb : neighbors)
        {
            if (nb < 0 || nb >= (_int)cellCount)
                continue;

            if (nodes[nb].closed)
                continue;

            float tentativeG = nodes[current].g + DistanceCells(current, nb);

            if (tentativeG < nodes[nb].g)
            {
                nodes[nb].parent = current;
                nodes[nb].g = tentativeG;
                nodes[nb].f = tentativeG + Heuristic(nb);

                open.push(OpenEntry{ nodes[nb].f, nb });
            }
        }
    }

    // 경로 없음
    return false;
}

_bool NavigationManager::BuildCenterWaypointsFromCellPath(const vector<_int>& cellPath, vector<_float3>& outWaypoints) const
{
    outWaypoints.clear();
    if (cellPath.empty())
        return false;

    outWaypoints.reserve(cellPath.size());

    for (_int cellIdx : cellPath)
    {
        if (cellIdx < 0 || cellIdx >= (_int)m_Cells.size())
            continue;

        outWaypoints.push_back(GetCellCenter(cellIdx));
    }

    return !outWaypoints.empty();
}

_bool NavigationManager::BuildFunnelWaypointsFromCellPath(_fvector startPos, _fvector goalPos, const vector<_int>& cellPath, vector<_float3>& outWaypoints) const
{
    outWaypoints.clear();

    if (cellPath.empty())
        return false;

    std::vector<Portal> portals;
    if (!BuildPortalsFromCells(startPos, goalPos, cellPath, portals))
        return false;

    _float3 start;
    XMStoreFloat3(&start, startPos);

    StringPullFunnel(start, portals, outWaypoints);

    return !outWaypoints.empty();
}

_bool NavigationManager::BuildMidWaypointsFromCellPath(_fvector startPos, _fvector goalPos, const vector<_int>& cellPath, vector<_float3>& outWaypoints) const
{
    outWaypoints.clear();
    if (cellPath.empty())
        return false;

    _float3 start, goal;
    XMStoreFloat3(&start, startPos);
    XMStoreFloat3(&goal, goalPos);

    if (cellPath.size() == 1)
    {
        outWaypoints.push_back(goal);
        return true;
    }

    outWaypoints.reserve(cellPath.size() + 1);

    for (size_t i = 0; i + 1 < cellPath.size(); ++i)
    {
        _int c0 = cellPath[i];
        _int c1 = cellPath[i + 1];

        _float3 v0, v1;
        if (!FindSharedEdge(c0, c1, v0, v1))
            return false;

        // 공유 엣지의 중앙점
        _float3 mid{
            (v0.x + v1.x) * 0.5f,
            (v0.y + v1.y) * 0.5f,
            (v0.z + v1.z) * 0.5f
        };

        outWaypoints.push_back(mid);
    }

    // 마지막에 정확한 목표 위치도 한 번 더 찍어줌
    outWaypoints.push_back(goal);

    return !outWaypoints.empty();
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

bool NavigationManager::FindNearestEdgePairXZ(const _float3 triA[3], const _float3 triB[3], LINETYPE& outEdgeA, LINETYPE& outEdgeB, _float3& outA0, _float3& outA1, _float3& outB0, _float3& outB1) const
{
    struct EdgeDef { LINETYPE e; _float3 a, b; };
    auto buildEdges = [&](const _float3 t[3], EdgeDef edges[3]) {
        edges[0] = { LINETYPE::AB, t[POINTTYPE::A], t[POINTTYPE::B] };
        edges[1] = { LINETYPE::BC, t[POINTTYPE::B], t[POINTTYPE::C] };
        edges[2] = { LINETYPE::CA, t[POINTTYPE::C], t[POINTTYPE::A] };
        };

    EdgeDef EA[3], EB[3];
    buildEdges(triA, EA);
    buildEdges(triB, EB);

    auto segSegDistXZ = [&](const _float3& a0, const _float3& a1,
        const _float3& b0, const _float3& b1)->float
        {
            // 엔드포인트→상대 세그먼트 거리의 최소값(대칭 4회)
            float d0 = DistPointToSegmentXZ(a0, b0, b1);
            float d1 = DistPointToSegmentXZ(a1, b0, b1);
            float d2 = DistPointToSegmentXZ(b0, a0, a1);
            float d3 = DistPointToSegmentXZ(b1, a0, a1);
            return min(min(d0, d1), min(d2, d3));
        };

    float best = FLT_MAX;
    int bi = -1, bj = -1;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float d = segSegDistXZ(EA[i].a, EA[i].b, EB[j].a, EB[j].b);
            if (d < best)
            {
                best = d; bi = i; bj = j;
            }
        }
    }
    if (bi < 0 || bj < 0) return false;

    // 에지 선택
    outEdgeA = EA[bi].e; outEdgeB = EB[bj].e;

    // 두 가지 짝 매칭 중 총합 거리가 더 작은 순서(A0↔B0, A1↔B1)로 정렬
    _float3 A0 = EA[bi].a, A1 = EA[bi].b;
    _float3 B0 = EB[bj].a, B1 = EB[bj].b;

    auto dXZ = [&](const _float3& p, const _float3& q) {
        float dx = p.x - q.x, dz = p.z - q.z; return sqrtf(dx * dx + dz * dz);
        };
    float pairSum1 = dXZ(A0, B0) + dXZ(A1, B1);
    float pairSum2 = dXZ(A0, B1) + dXZ(A1, B0);

    if (pairSum1 <= pairSum2) { outA0 = A0; outA1 = A1; outB0 = B0; outB1 = B1; }
    else { outA0 = A0; outA1 = A1; outB0 = B1; outB1 = B0; }

    return true;
}

_float3 NavigationManager::GetCellCenter(_int iCellIndex) const
{
    _float3 a, b, c;
    XMStoreFloat3(&a, m_Cells[iCellIndex]->GetPoint(POINTTYPE::A));
    XMStoreFloat3(&b, m_Cells[iCellIndex]->GetPoint(POINTTYPE::B));
    XMStoreFloat3(&c, m_Cells[iCellIndex]->GetPoint(POINTTYPE::C));

    _float3 center{
        (a.x + b.x + c.x) / 3.f,
        (a.y + b.y + c.y) / 3.f,
        (a.z + b.z + c.z) / 3.f,
    };
    return center;
}

void NavigationManager::GetNeighborIndices(_int iCellIndex, vector<_int>& outNeighbors) const
{
    outNeighbors.clear();
    Cell* cell = m_Cells[iCellIndex];

    for (_int i = 0; i < 3; ++i)
    {
        _int n = cell->GetNeighborIndex(LINETYPE(i)); // 네 쪽 함수 이름에 맞게 수정
        if (n >= 0)
            outNeighbors.push_back(n);
    }
}

_bool NavigationManager::FindSharedEdge(_int cellA, _int cellB, _float3& outV0, _float3& outV1) const
{
    if (cellA < 0 || cellB < 0 ||
        cellA >= (_int)m_Cells.size() ||
        cellB >= (_int)m_Cells.size())
        return false;

    const Cell* a = m_Cells[cellA];
    const Cell* b = m_Cells[cellB];

    _float3 aPts[3];
    _float3 bPts[3];

    XMStoreFloat3(&aPts[0], a->GetPoint(POINTTYPE::A));
    XMStoreFloat3(&aPts[1], a->GetPoint(POINTTYPE::B));
    XMStoreFloat3(&aPts[2], a->GetPoint(POINTTYPE::C));

    XMStoreFloat3(&bPts[0], b->GetPoint(POINTTYPE::A));
    XMStoreFloat3(&bPts[1], b->GetPoint(POINTTYPE::B));
    XMStoreFloat3(&bPts[2], b->GetPoint(POINTTYPE::C));

    // 공통되는 두 점 찾기 (epsilon 비교)
    int foundIdxA[2] = { -1, -1 };
    int foundIdxB[2] = { -1, -1 };
    int foundCount = 0;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (NearlyEqual3(aPts[i], bPts[j]))
            {
                if (foundCount < 2)
                {
                    foundIdxA[foundCount] = i;
                    foundIdxB[foundCount] = j;
                }
                ++foundCount;
            }
        }
    }

    if (foundCount < 2)
        return false;

    // 두 점이 공통 엣지
    outV0 = aPts[foundIdxA[0]];
    outV1 = aPts[foundIdxA[1]];
    return true;
}

_bool NavigationManager::BuildPortalsFromCells(_fvector startPos, _fvector goalPos, const std::vector<_int>& cellPath, std::vector<Portal>& outPortals) const
{
    outPortals.clear();

    if (cellPath.empty())
        return false;

    _float3 start;
    XMStoreFloat3(&start, startPos);

    // 셀 경로가 1개뿐이면 포털 없이 바로 goal로 처리
    if (cellPath.size() == 1)
    {
        Portal p;
        _float3 goal;
        XMStoreFloat3(&goal, goalPos);
        p.left = goal;
        p.right = goal;
        outPortals.push_back(p);
        return true;
    }

    outPortals.reserve(cellPath.size());

    for (size_t i = 0; i + 1 < cellPath.size(); ++i)
    {
        const _int c0 = cellPath[i];
        const _int c1 = cellPath[i + 1];

        _float3 v0, v1;
        if (!FindSharedEdge(c0, c1, v0, v1))
            return false; // nav 데이터가 깨졌거나 이웃 정의가 잘못된 경우

        // start 기준으로 left/right 정렬
        // TriArea2_XZ(start, v0, v1) > 0 이면 v1이 오른쪽 쪽에 있는 셈
        // (winding에 따라 부호가 바뀔 수 있으니 필요하면 나중에 뒤집어줘도 됨)
        Portal p;

        if (TriArea2_XZ(start, v0, v1) >= 0.f)
        {
            p.left = v0;
            p.right = v1;
        }
        else
        {
            p.left = v1;
            p.right = v0;
        }

        outPortals.push_back(p);
    }

    // 마지막 포털로 goal을 degenerate portal로 추가
    {
        Portal p;
        _float3 goal;
        XMStoreFloat3(&goal, goalPos);
        p.left = goal;
        p.right = goal;
        outPortals.push_back(p);
    }

    return !outPortals.empty();
}

void NavigationManager::StringPullFunnel(const _float3& start, const std::vector<Portal>& portals, std::vector<_float3>& outPoints) const
{
    outPoints.clear();
    if (portals.empty())
    {
        outPoints.push_back(start);
        return;
    }

    outPoints.reserve(portals.size() + 1);
    outPoints.push_back(start);

    // apex/left/right 초기 상태
    _float3 apex = start;
    _float3 left = portals[0].left;
    _float3 right = portals[0].right;

    int apexIndex = 0;
    int leftIndex = 0;
    int rightIndex = 0;

    for (int i = 1; i < (int)portals.size(); ++i)
    {
        const _float3& newLeft = portals[i].left;
        const _float3& newRight = portals[i].right;

        // ----- 왼쪽 갱신 -----
        if (TriArea2_XZ(apex, left, newLeft) >= 0.f)
        {
            left = newLeft;
            leftIndex = i;
        }

        // 왼쪽 갱신으로 funnel이 뒤집혔는지 검사
        if (TriArea2_XZ(apex, right, left) < 0.f)
        {
            // apex -> right 가 확정된 코너
            outPoints.push_back(right);

            apex = right;
            apexIndex = rightIndex;

            // 새 apex 기준으로 funnel 재설정
            left = apex;
            right = apex;
            leftIndex = apexIndex;
            rightIndex = apexIndex;

            // i를 apexIndex부터 다시
            i = apexIndex;
            continue;
        }

        // ----- 오른쪽 갱신 -----
        if (TriArea2_XZ(apex, right, newRight) <= 0.f)
        {
            right = newRight;
            rightIndex = i;
        }

        // 오른쪽 갱신으로 funnel이 뒤집혔는지 검사
        if (TriArea2_XZ(apex, right, left) < 0.f)
        {
            // apex -> left 가 확정된 코너
            outPoints.push_back(left);

            apex = left;
            apexIndex = leftIndex;

            left = apex;
            right = apex;
            leftIndex = apexIndex;
            rightIndex = apexIndex;

            i = apexIndex;
            continue;
        }
    }

    // 마지막 목적지(마지막 포털의 left/right) 추가
    const _float3& goal = portals.back().left;
    outPoints.push_back(goal);
}
