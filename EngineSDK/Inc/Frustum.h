#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class Frustum final : public Base
{
	Frustum();
	virtual ~Frustum() = default;

public:
	HRESULT Initialize();
	void Update();
	_bool IsIn_WorldSpace(_fvector vWorldPos, _float fRadius);

	static Frustum* Create();
	virtual void Free() override;
private:
	void Make_Planes(const _float3* pPoints, _float4* pPlanes);

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	_float3			m_vOriginalPoints[8] = {};
	_float4			m_vWorldPlanes[6] = {};
};

NS_END