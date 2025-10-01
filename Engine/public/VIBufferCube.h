#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferCube final : public VertexIndexBuffer
{
private:
	VIBufferCube();
	VIBufferCube(const VIBufferCube& Prototype);
	virtual ~VIBufferCube() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	static VIBufferCube* Create();
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END