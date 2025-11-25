#pragma once

#include "VIBufferInstancing.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferInstancingRect final : public VIBufferInstancing
{
public:
	typedef struct tagRectInstance final : public VIBufferInstancing::INSTANCE_DESC
	{
		_float3			vPivot;
		_float2			vSpeed;
		_float2			vLifeTime;
		_bool			isLoop;
		_bool			repeatable = false;
		_float2			repeatDuration = { 0.f, 0.f };
	}RECT_INSTANCE_DESC;

private:
	VIBufferInstancingRect();
	VIBufferInstancingRect(const VIBufferInstancingRect& Prototype);
	virtual ~VIBufferInstancingRect() = default;

public:
	virtual HRESULT InitializePrototype(const INSTANCE_DESC* pDesc) override;
	virtual HRESULT Initialize(void* pArg) override;

	void Drop(_float fTimeDelta);
	void Spread(_float fTimeDelta);
	void SpreadRepeat(_float fTimeDelta);

	static VIBufferInstancingRect* Create(const INSTANCE_DESC* pDesc);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;

private:
	VTXINSTANCEPARTICLE* m_pInstanceVertices = { nullptr };
	_float* m_pSpeeds = { nullptr };
	_bool					m_isLoop = {};
	_float3					m_vPivot = {};

	_bool m_repeatable = false;
	_float2 m_vRepeatDurationRange = _float2{ 0.f, 0.f };
	_float* m_pRepeatDurations = { nullptr };
};

NS_END