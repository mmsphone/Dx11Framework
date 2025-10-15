#include "Navigation.h"

#include "Cell.h"
#include "Shader.h"
#include "EngineUtility.h"

const _float4x4* Navigation::m_pParentMatrix = { nullptr };

Navigation::Navigation()
    : Component{ }
{
}

Navigation::Navigation(const Navigation& Prototype)
    : Component{ Prototype }
    , m_iCurrentCellIndex{ Prototype.m_iCurrentCellIndex }
    , m_Cells{ Prototype.m_Cells }
#ifdef _DEBUG
    , m_pShader{ Prototype.m_pShader }
#endif
{
    for (auto& pCell : m_Cells)
        SafeAddRef(pCell);

#ifdef _DEBUG
    SafeAddRef(m_pShader);
#endif
}

HRESULT Navigation::InitializePrototype(const _tchar* pNavigationDataFile)
{
    _ulong			dwByte = { 0 };
    HANDLE			hFile = CreateFile(pNavigationDataFile, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    _float3         vPoints[POINT_END] = {};

    while (true)
    {
        ReadFile(hFile, vPoints, sizeof(_float3) * POINT_END, &dwByte, nullptr);
        if (0 == dwByte)
            break;

        Cell* pCell = Cell::Create(vPoints, m_Cells.size());
        if (nullptr == pCell)
            return E_FAIL;

        m_Cells.push_back(pCell);
    }

    CloseHandle(hFile);

    SetNeighbors();

#ifdef _DEBUG
    m_pShader = Shader::Create( TEXT("../bin/Shader/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;
#endif

    return S_OK;
}

HRESULT Navigation::Initialize(void* pArg)
{
    if (nullptr != pArg)
        m_iCurrentCellIndex = static_cast<NAVIGATION_DESC*>(pArg)->iCurrentCellIndex;

    return S_OK;
}

void Navigation::Update(const _float4x4* pParentMatrix)
{
    m_pParentMatrix = pParentMatrix;
}

_bool Navigation::isMove(_fvector vResultPos)
{
    if (-1 == m_iCurrentCellIndex)
        return false;

    _vector     vPosition = XMVector3TransformCoord(vResultPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pParentMatrix)));

    _int        iNeighborIndex = { -1 };

    if (true == m_Cells[m_iCurrentCellIndex]->isIn(vPosition, &iNeighborIndex))
    {
        return true;
    }
    else
    {
        if (-1 != iNeighborIndex)
        {
            for (;;)
            {
                if (true == m_Cells[iNeighborIndex]->isIn(vPosition, &iNeighborIndex))
                    break;

                if (-1 == iNeighborIndex)
                    return false;
            }

            m_iCurrentCellIndex = iNeighborIndex;

            return true;
        }
        else
            return false;
    }
}

_vector Navigation::SetOnNavigation(_fvector vWorldPos)
{
    _vector     vPosition = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pParentMatrix)));

    return XMVectorSetY(vPosition, m_Cells[m_iCurrentCellIndex]->ComputeHeight(vPosition));
}

const vector<class Cell*>& Navigation::GetCells()
{
    return m_Cells;
}

void Navigation::AddCell(_float3* pPoints)
{
    if (pPoints == nullptr) return;

    _uint iCellIndex = (m_Cells.empty()) ? 0 : (m_Cells.back()->GetIndex() + 1);

    Cell* pCell = Cell::Create(pPoints, iCellIndex);
    if (pCell == nullptr) return;

    m_Cells.push_back(pCell);
}

void Navigation::RemoveRecentCell()
{
    if (m_Cells.empty()) return;

    SafeRelease(m_Cells.back());
    m_Cells.pop_back();
}

void Navigation::ClearCells()
{
    if (m_Cells.empty()) return;

    for (auto& cell : m_Cells)
        SafeRelease(cell);
    m_Cells.clear();
}

void Navigation::SaveCells(const _tchar* pNavigationDataFile)
{
    if (m_Cells.empty())
        return;

    HANDLE hFile = CreateFile(
        pNavigationDataFile,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        MSG_BOX("Failed to Create Navigation Save File");
        return;
    }

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


#ifdef _DEBUG
HRESULT Navigation::Render()
{
    if (FAILED(m_pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(m_pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::PROJECTION))))
        return E_FAIL;

    _float4x4 WorldMatrix{};
    if(m_pParentMatrix == nullptr)
        XMStoreFloat4x4(&WorldMatrix,XMMatrixIdentity()); 
    else
        WorldMatrix = *m_pParentMatrix;

    _float4     vColor = {};

    if (-1 == m_iCurrentCellIndex)
    {
        vColor = _float4(1.f, 1.f, 1.f, 1.f);
        m_pShader->BindRawValue("g_vColor", &vColor, sizeof(_float4));

        if (FAILED(m_pShader->BindMatrix("g_WorldMatrix", &WorldMatrix)))
            return E_FAIL;

        m_pShader->Begin(0);
        for (auto& pCell : m_Cells)
        {
            pCell->Render();
        }
    }
    else
    {
        vColor = _float4(1.f, 0.0f, 0.f, 1.f);
        m_pShader->BindRawValue("g_vColor", &vColor, sizeof(_float4));

        WorldMatrix._42 += 0.1f;
        if (FAILED(m_pShader->BindMatrix("g_WorldMatrix", &WorldMatrix)))
            return E_FAIL;

        m_pShader->Begin(0);
        m_Cells[m_iCurrentCellIndex]->Render();
    }

    return S_OK;
}
#endif

void Navigation::SetNeighbors()
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

Navigation* Navigation::Create( const _tchar* pNavigationDataFile)
{
    Navigation* pInstance = new Navigation();

    if (FAILED(pInstance->InitializePrototype(pNavigationDataFile)))
    {
        MSG_BOX("Failed to Created : Navigation");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Component* Navigation::Clone(void* pArg)
{
    Navigation* pInstance = new Navigation(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Navigation");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Navigation::Free()
{
    __super::Free();

    for (auto& pCell : m_Cells)
        SafeRelease(pCell);
    m_Cells.clear();

#ifdef _DEBUG
    SafeRelease(m_pShader);
#endif
}
