#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferCell final : public VertexIndexBuffer
{
private:
	VIBufferCell();
	VIBufferCell(const VIBufferCell& Prototype);
	virtual ~VIBufferCell() = default;

public:
	virtual HRESULT InitializePrototype(const _float3* pPoints);
	virtual HRESULT Initialize(void* pArg) override;

	static VIBufferCell* Create(const _float3* pPoints);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END