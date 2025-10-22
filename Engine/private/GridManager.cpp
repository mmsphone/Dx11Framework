#include "GridManager.h"

#include "EngineUtility.h"

GridManager::GridManager()
    :m_pEngineUtility{EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT GridManager::Initialize(_uint iNumCells, _float fCellSize)
{
    m_iNumCells = iNumCells;
    m_fCellSize = fCellSize;

    m_pEngineUtility = EngineUtility::GetInstance();
    SafeAddRef(m_pEngineUtility);

#ifdef _DEBUG
    auto pDevice = m_pEngineUtility->GetDevice();
    auto pContext = m_pEngineUtility->GetContext();

    m_pBatch = new PrimitiveBatch<VertexPositionColor>(pContext);
    m_pEffect = new BasicEffect(pDevice);
    m_pEffect->SetVertexColorEnabled(true);

    const void* pShaderCode = nullptr;
    size_t length = 0;
    m_pEffect->GetVertexShaderBytecode(&pShaderCode, &length);
    HRESULT hr = pDevice->CreateInputLayout(
        VertexPositionColor::InputElements,
        VertexPositionColor::InputElementCount,
        pShaderCode,
        length,
        &m_pInputLayout
    );
    if (FAILED(hr))
        return hr;
#endif

    return S_OK;
}

#ifdef _DEBUG
void GridManager::Render()
{
    if (!m_GridVisible)
        return;

    auto pContext = m_pEngineUtility->GetContext();

    XMMATRIX view = XMLoadFloat4x4(m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::VIEW));
    XMMATRIX proj = XMLoadFloat4x4(m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::PROJECTION));

    m_pEffect->SetView(view);
    m_pEffect->SetProjection(proj);
    m_pEffect->Apply(pContext);
    pContext->IASetInputLayout(m_pInputLayout);
    m_pBatch->Begin();

    float half = (m_iNumCells * m_fCellSize) * 0.5f;

    for (int i = 0; i <= static_cast<int>(m_iNumCells); ++i)
    {
        float pos = -half + i * m_fCellSize;

        XMFLOAT4 colorX = (fabs(pos) < 0.001f) ? XMFLOAT4(1.f, 0.f, 0.f, 1.f) : XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);
        XMFLOAT4 colorZ = (fabs(pos) < 0.001f) ? XMFLOAT4(0.f, 0.f, 1.f, 1.f) : XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f);

        // X축 방향
        m_pBatch->DrawLine(
            VertexPositionColor(XMFLOAT3(-half, 0.f, pos), colorX),
            VertexPositionColor(XMFLOAT3(half, 0.f, pos), colorX)
        );

        // Z축 방향
        m_pBatch->DrawLine(
            VertexPositionColor(XMFLOAT3(pos, 0.f, -half), colorZ),
            VertexPositionColor(XMFLOAT3(pos, 0.f, half), colorZ)
        );
    }

    m_pBatch->End();
}

#endif

void GridManager::SetVisible(_bool bVisible)
{
    m_GridVisible = bVisible;
}

_bool GridManager::IsVisible()
{
    return m_GridVisible;
}

_float GridManager::GetGridCellSize()
{
    return m_fCellSize;
}

void GridManager::SetGridCellSize(_float cellSize)
{
    m_fCellSize = cellSize;
}

_uint GridManager::GetNumGridCells()
{
    return m_iNumCells;
}

void GridManager::SetNumGridCells(_uint iNumGridCells)
{
    m_iNumCells = iNumGridCells;
}

GridManager* GridManager::Create(_uint iNumCells, _float fCellSize)
{
    GridManager* pInstance = new GridManager();
    if (FAILED(pInstance->Initialize(iNumCells, fCellSize)))
    {
        MSG_BOX("Failed to Create: GridManager");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void GridManager::Free()
{
    __super::Free();
    SafeRelease(m_pEngineUtility);

#ifdef _DEBUG
    SafeDelete(m_pBatch);
    SafeDelete(m_pEffect);
    SafeRelease(m_pInputLayout);
#endif
}
