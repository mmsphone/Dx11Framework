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
	void SetNeighbor(LINETYPE eLine, Cell* pNeighbor);

	HRESULT Initialize(const _float3* pPoints, _uint iIndex);
	_bool isIn(_fvector vResultPos, _int* pNeighborIndex);
	_bool ComparePoints(_fvector vSour, _fvector vDest);
	_float ComputeHeight(_fvector vResultPos);

	static Cell* Create(const _float3* pPoints, _uint iIndex);
	virtual void Free() override;
#ifdef _DEBUG
	HRESULT Render();

private:
	class VIBufferCell* m_pDebugBuffer = { };
#endif

private:
	_uint			m_iIndex = {};
	_float3			m_vPoints[POINT_END] = {};
	_float3			m_vNormals[LINE_END] = {};

	_int			m_iNeighborIndices[3] = { -1, -1, -1 };
};

NS_END