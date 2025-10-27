#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL NavigationManager final : public Base
{
private:	
	NavigationManager();
	virtual ~NavigationManager() = default;

public:
	HRESULT Initialize();
#ifdef _DEBUG
	HRESULT Render();
#endif

	void AddCell(_float3* pPoints);
	void AddTempPoint(const _float3& point);
	void RemoveRecentCell();
	void ClearCells();

	void SaveCells(const _char* pNavigationDataFile);
	void LoadCells(const _char* pNavigationDataFile);

	_bool IsInCell(_fvector vWorldPos, _int* pOutCellIndex = nullptr);
	_bool SetHeightOnCell(_fvector vWorldPos, _vector* pOutAdjustedPos);

	const vector<class Cell*>& GetCells() const;

	static NavigationManager* Create();
	virtual void Free() override;

private:
	void SetNeighbors();
	void SortPointsClockWise(std::vector<_float3>& points);

private:
	class EngineUtility* m_pEngineUtility = nullptr;
	vector<class Cell*>	m_Cells = {};
	vector<_float3> m_TempPoints = {};
#ifdef _DEBUG
	class Shader* m_pShader = { nullptr };
#endif
};

NS_END