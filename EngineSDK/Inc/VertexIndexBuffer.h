#pragma once

#include "Component.h"

NS_BEGIN(Engine)


class ENGINE_DLL VertexIndexBuffer abstract : public Component
{
protected:
	VertexIndexBuffer();
	VertexIndexBuffer(const VertexIndexBuffer& Prototype);
	virtual ~VertexIndexBuffer() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT BindBuffers();
	virtual HRESULT Render();

	virtual Component* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	ID3D11Buffer* m_pVertexBuffer = { nullptr };
	ID3D11Buffer* m_pIndexBuffer = { nullptr };

	_uint					m_iNumVertexBuffers = {};
	_uint					m_iNumVertices = {};
	_uint					m_iNumIndices = {};

	_uint					m_iVertexStride = {};
	_uint					m_iIndexStride = {};

	DXGI_FORMAT				m_eIndexFormat = {};
	D3D_PRIMITIVE_TOPOLOGY	m_ePrimitive = {};
};

NS_END