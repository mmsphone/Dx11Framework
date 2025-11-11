#pragma once

#include "VIBufferInstancing.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBufferInstancingRect final : public VIBufferInstancing
{
public:
	typedef struct tagRectInstance final : public VIBufferInstancing::INSTANCE_DESC
	{
		_float3			vPivot;
		_float2			vSpeed;
		_float2			vLifeTime;
		_bool			isLoop;
	}RECT_INSTANCE_DESC;

private:
	CVIBufferInstancingRect();
	CVIBufferInstancingRect(const CVIBufferInstancingRect& Prototype);
	virtual ~CVIBufferInstancingRect() = default;

public:
	virtual HRESULT InitializePrototype(const INSTANCE_DESC* pDesc) override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	void Drop(_float fTimeDelta);
	void Spread(_float fTimeDelta);

private:
	VTXINSTANCEPARTICLE* m_pInstanceVertices = { nullptr };
	_float* m_pSpeeds = { nullptr };
	_bool					m_isLoop = {};
	_float3					m_vPivot = {};



public:
	static CVIBufferInstancingRect* Create(const INSTANCE_DESC* pDesc);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END