#pragma once

#include "VIBufferInstancing.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferInstancingPoint final : public VIBufferInstancing
{
public:
	typedef struct tagPointInstance final : public VIBufferInstancing::INSTANCE_DESC
	{
		_float3			vPivot;
		_float2			vSpeed;
		_float2			vLifeTime;
		_bool			isLoop;
	}POINT_INSTANCE_DESC;

private:
	VIBufferInstancingPoint();
	VIBufferInstancingPoint(const VIBufferInstancingPoint& Prototype);
	virtual ~VIBufferInstancingPoint() = default;

public:
	virtual HRESULT InitializePrototype(const INSTANCE_DESC* pDesc) override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT BindBuffers() override;
	virtual HRESULT Render() override;

public:
	void Drop(_float fTimeDelta);
	void Spread(_float fTimeDelta);

private:
	VTXINSTANCEPARTICLE* m_pInstanceVertices = { nullptr };
	_float* m_pSpeeds = { nullptr };
	_bool					m_isLoop = {};
	_float3					m_vPivot = {};

public:
	static VIBufferInstancingPoint* Create(const INSTANCE_DESC* pDesc);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END