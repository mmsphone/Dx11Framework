#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL Cell final : public Base
{
private:
	Cell();
	virtual ~Cell() = default;

public:
	_vector GetPoint(POINTTYPE ePoint) const;
	_uint GetIndex() const;
	_uint GetNeighborIndex(LINETYPE eLine);
	void SetNeighborIndex(LINETYPE eLine, _uint iNeighborIndex);

	HRESULT Initialize(const _float3* pPoints, _uint iIndex);
	_bool isIn(_fvector vResultPos, _int* pNeighborIndex);
	_bool ComparePoints(_fvector vSour, _fvector vDest);
	_bool ComparePointsEps(_fvector vSour, _fvector vDest, _float eps);
	_float ComputeHeight(_fvector vResultPos);

	static Cell* Create(const _float3* pPoints, _uint iIndex);
	virtual void Free() override;
#ifdef _DEBUG
	HRESULT Render();
#endif

private:
	_bool AlmostEqual3D(const _float3& a, const _float3& b, float eps);

private:
	_uint			m_iIndex = {};
	_float3			m_vPoints[POINT_END] = {};
	_float3			m_vNormals[LINE_END] = {};

	_int			m_iNeighborIndices[3] = { -1, -1, -1 };

#ifdef _DEBUG
	class VIBufferCell* m_pDebugBuffer = { };
#endif
};

NS_END