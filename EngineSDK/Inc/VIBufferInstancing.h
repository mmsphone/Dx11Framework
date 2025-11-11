#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferInstancing abstract : public VertexIndexBuffer
{
public:
	typedef struct tagInstanceDesc
	{
		_uint		iNumInstance;
		_float2		vScale;
		_float3		vCenter;
		_float3		vRange;
	}INSTANCE_DESC;
protected:
	VIBufferInstancing();
	VIBufferInstancing(const VIBufferInstancing& Prototype);
	virtual ~VIBufferInstancing() = default;

public:
	virtual HRESULT InitializePrototype(const INSTANCE_DESC* pDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT BindBuffers() override;
	virtual HRESULT Render() override;
protected:
	ID3D11Buffer* m_pVBInstance = { };
	_uint		  m_iInstanceVertexStride = {};
	_uint		  m_iNumInstance = {};
	_uint		  m_iIndexCountPerInstance = {};

	D3D11_BUFFER_DESC m_InstanceDesc = {};

public:
	virtual Component* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END