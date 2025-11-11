#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class GridManager final : public Base
{
	GridManager();
	virtual ~GridManager() = default;

public:
    HRESULT Initialize(_uint iNumCells = 200, _float fCellSize = 2.0f);
#ifdef _DEBUG
    void Render();

    void SetVisible(_bool bGridVisible);
    _bool IsVisible() const;
#endif 
    _float GetGridCellSize() const;
    void SetGridCellSize(_float cellSize);
    _uint GetNumGridCells() const;
    void SetNumGridCells(_uint iNumGridCells);
    void SetMarkerPosition(const _float3& vPos);
    _float3 GetMarkerPosition() const;
    void ClearMarker();
    _bool IsMark() const;

    static GridManager* Create(_uint iNumCells = 200, _float fCellSize = 2.0f);
    virtual void Free() override;

private:
    class EngineUtility* m_pEngineUtility = nullptr;

    _uint m_iNumCells = 0;
    _float m_fCellSize = 0.f;

    _bool m_isMark = false;
    _float3 m_vMarkerPosition = { 0.f,0.f,0.f };

#ifdef _DEBUG
    class PrimitiveBatch<VertexPositionColor>* m_pBatch = nullptr;
    class BasicEffect* m_pEffect = nullptr;
    ID3D11InputLayout* m_pInputLayout = nullptr;
    _bool m_GridVisible = true;
#endif
};

NS_END