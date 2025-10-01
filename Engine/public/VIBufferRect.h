#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferRect final : public VertexIndexBuffer
{
private:
	VIBufferRect();
	VIBufferRect(const VIBufferRect& Prototype);
	virtual ~VIBufferRect() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	static VIBufferRect* Create();
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END