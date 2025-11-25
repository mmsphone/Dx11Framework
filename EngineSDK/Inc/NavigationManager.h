#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL NavigationManager final : public Base
{
public:
	struct AStarNode
	{
		_int  parent = -1;
		_float g = FLT_MAX;
		_float f = FLT_MAX;
		_bool closed = false;
	};
	struct OpenEntry
	{
		float f;
		_int  index;
	};
	struct CompareOpenEntry
	{
		bool operator()(const OpenEntry& a, const OpenEntry& b) const
		{
			return a.f > b.f; // f 작은 게 top으로 오게
		}
	};
	struct Portal
	{
		_float3 left;
		_float3 right;
	};
private:	
	NavigationManager();
	virtual ~NavigationManager() = default;

public:
	HRESULT Initialize();
#ifdef _DEBUG
	HRESULT Render();
#endif
	
	void AddTempPoint(const _float3& point);
	void ClearTempPoints();
	void RemoveCell(_int cellIndex);
	void RemoveRecentCell();
	void ClearCells();

	void SaveCells(const _char* pNavigationDataFile);
	void LoadCells(const _char* pNavigationDataFile);

	_bool IsInCell(_fvector vWorldPos, _int* pOutCellIndex = nullptr);
	_float GetHeightPosOnCell(_vector* pPos, const _int& pCellIndex);
	_bool SetHeightOnCell(_fvector vWorldPos, _vector* pOutAdjustedPos);
	_bool GetSlideVectorOnCell(_fvector pos, _fvector delta, _int cellIndex, _vector* outSlideVector) const;
	const vector<class Cell*>& GetCells() const;

	_bool Edit_AddTriangleOnEdge(_int cellId, _fvector pickedPoint, _float weldEps);
	_bool Edit_AddTriangleAtSharedVertex(_int cellA, _int cellB, _float weldEps);
	_bool RandomPointAround(_fvector center, _float radius, _float3* outPos, _uint maxTrials = 64);

	_bool FindPath(_fvector startPos, _fvector goalPos, vector<_int>& outCellPath);
	_bool BuildCenterWaypointsFromCellPath(const vector<_int>& cellPath, vector<_float3>& outWaypoints) const;
	_bool BuildFunnelWaypointsFromCellPath(_fvector startPos, _fvector goalPos, const vector<_int>& cellPath, vector<_float3>& outWaypoints) const;
	_bool BuildMidWaypointsFromCellPath(_fvector startPos, _fvector goalPos, const vector<_int>& cellPath, vector<_float3>& outWaypoints) const;

	static NavigationManager* Create();
	virtual void Free() override;

private:
	void AddCell(_float3* pPoints);
	void SetNeighborsForNewCell(_int newCellIndex);
	void SortPointsClockWise(std::vector<_float3>& points);
	void  EdgeEndpoints(const _float3 tri[3], LINETYPE e, _float3& outA, _float3& outB) const;
	float DistPointToSegmentXZ(const _float3& p, const _float3& a, const _float3& b) const;
	void  SnapIfNear(_float3& inoutP, const _float3& target, float eps) const;
	bool FindNearestEdgePairXZ(const _float3 triA[3], const _float3 triB[3], LINETYPE& outEdgeA, LINETYPE& outEdgeB, _float3& outA0, _float3& outA1, _float3& outB0, _float3& outB1) const;

	_float3 GetCellCenter(_int iCellIndex) const;
	void GetNeighborIndices(_int iCellIndex, vector<_int>& outNeighbors) const;
	_bool FindSharedEdge(_int cellA, _int cellB, _float3& outV0, _float3& outV1) const;
	_bool BuildPortalsFromCells(_fvector startPos,_fvector goalPos,const std::vector<_int>& cellPath,std::vector<Portal>& outPortals) const;
	void StringPullFunnel(const _float3& start,const std::vector<Portal>& portals,std::vector<_float3>& outPoints) const;

private:
	class EngineUtility* m_pEngineUtility = nullptr;
	vector<class Cell*>	m_Cells = {};
	vector<_float3> m_TempPoints = {};
#ifdef _DEBUG
	class Shader* m_pShader = { nullptr };
#endif

	_int       m_EditingCell = -1;
};

NS_END