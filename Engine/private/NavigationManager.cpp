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
    if (FAILED(m_pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::PROJECTION))))
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

void NavigationManager::AddCell(_float3* pPoints)
{
    if (pPoints == nullptr) 
        return;

    _uint iCellIndex = (m_Cells.empty()) ? 0 : (m_Cells.back()->GetIndex() + 1);

    Cell* pCell = Cell::Create(pPoints, iCellIndex);
    if (pCell == nullptr) 
        return;

    m_Cells.push_back(pCell);
    SetNeighbors();
}

void NavigationManager::AddTempPoint(const _float3& point)
{
    m_TempPoints.push_back(point);
    if (m_TempPoints.size() == 3)
    {
        SortPointsClockWise(m_TempPoints);
        AddCell(m_TempPoints.data());
        m_TempPoints.clear();
    }
}

void NavigationManager::RemoveRecentCell()
{
    if (m_Cells.empty()) 
        return;

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
}

void NavigationManager::SaveCells(const _char* pNavigationDataFile)
{
    if (m_Cells.empty())
        return;

    HANDLE hFile = CreateFileA(
        pNavigationDataFile,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    _ulong dwByte = 0;

    for (auto& pCell : m_Cells)
    {
        _float3 vPoints[POINT_END];
        XMStoreFloat3(&vPoints[0], pCell->GetPoint(POINTTYPE::A));
        XMStoreFloat3(&vPoints[1], pCell->GetPoint(POINTTYPE::B));
        XMStoreFloat3(&vPoints[2], pCell->GetPoint(POINTTYPE::C));;

        WriteFile(hFile, vPoints, sizeof(_float3) * POINT_END, &dwByte, nullptr);
    }

    CloseHandle(hFile);
}

void NavigationManager::LoadCells(const _char* pNavigationDataFile)
{
    HANDLE			hFile = CreateFileA(pNavigationDataFile, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;
    
    _float3         vPoints[POINT_END] = {};
    _ulong			dwByte = { 0 };

    while (true)
    {
        ReadFile(hFile, vPoints, sizeof(_float3) * POINT_END, &dwByte, nullptr);
        if (0 == dwByte)
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

void NavigationManager::SetNeighbors()
{
    for (auto& pSourCell : m_Cells)
    {
        for (auto& pDestCell : m_Cells)
        {
            if (pSourCell == pDestCell)
                continue;

            if (true == pDestCell->ComparePoints(pSourCell->GetPoint(POINTTYPE::A), pSourCell->GetPoint(POINTTYPE::B)))
            {
                pSourCell->SetNeighbor(LINETYPE::AB, pDestCell);
            }

            if (true == pDestCell->ComparePoints(pSourCell->GetPoint(POINTTYPE::B), pSourCell->GetPoint(POINTTYPE::C)))
            {
                pSourCell->SetNeighbor(LINETYPE::BC, pDestCell);
            }

            if (true == pDestCell->ComparePoints(pSourCell->GetPoint(POINTTYPE::C), pSourCell->GetPoint(POINTTYPE::A)))
            {
                pSourCell->SetNeighbor(LINETYPE::CA, pDestCell);
            }
        }
    }
}

void NavigationManager::SortPointsClockWise(std::vector<_float3>& pts)
{
    if (pts.size() != 3) return;
    _float3 center{
        (pts[0].x + pts[1].x + pts[2].x) / 3.f,
        (pts[0].y + pts[1].y + pts[2].y) / 3.f,
        (pts[0].z + pts[1].z + pts[2].z) / 3.f
    };

    std::sort(pts.begin(), pts.end(), [&](const _float3& a, const _float3& b)
        {
            float a1 = atan2f(a.z - center.z, a.x - center.x);
            float a2 = atan2f(b.z - center.z, b.x - center.x);
            return a1 > a2;
        });
}