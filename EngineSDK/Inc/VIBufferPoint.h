#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferPoint final : public VertexIndexBuffer
{
public:
	typedef struct VIBufferPointDesc {
		_float3* pPoint = {};
	}VIBUFFER_POINT_DESC;
private:
	VIBufferPoint();
	VIBufferPoint(const VIBufferPoint& Prototype);
	virtual ~VIBufferPoint() = default;

public:
	virtual HRESULT InitializePrototype();
	virtual HRESULT Initialize(void* pArg) override;

	static VIBufferPoint* Create();
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END