#pragma once

#include "VertexIndexBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL VIBufferTerrain final : public VertexIndexBuffer
{
private:
	VIBufferTerrain();
	VIBufferTerrain(const VIBufferTerrain& Prototype);
	virtual ~VIBufferTerrain() = default;

public:
	virtual HRESULT InitializePrototype(const _tchar* pHeightMapFilePath);
	virtual HRESULT Initialize(void* pArg) override;

	static VIBufferTerrain* Create(const _tchar* pHeightMapFilePath);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
private:
	_uint			m_iNumVerticesX = {};
	_uint			m_iNumVerticesZ = {};
};

NS_END