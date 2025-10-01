#include "VertexIndexBuffer.h"
#include "EngineUtility.h"

VertexIndexBuffer::VertexIndexBuffer()
    :Component{}
{
}

VertexIndexBuffer::VertexIndexBuffer(const VertexIndexBuffer& Prototype)
    :Component{ Prototype }
	, m_pVertexBuffer{ Prototype.m_pVertexBuffer }
	, m_pIndexBuffer{ Prototype.m_pIndexBuffer }
	, m_iNumVertexBuffers{ Prototype.m_iNumVertexBuffers }
	, m_iNumVertices{ Prototype.m_iNumVertices }
	, m_iNumIndices{ Prototype.m_iNumIndices }
	, m_iVertexStride{ Prototype.m_iVertexStride }
	, m_iIndexStride{ Prototype.m_iIndexStride }
	, m_eIndexFormat{ Prototype.m_eIndexFormat }
	, m_ePrimitive{ Prototype.m_ePrimitive }
{
	SafeAddRef(m_pVertexBuffer);
	SafeAddRef(m_pIndexBuffer);
}

HRESULT VertexIndexBuffer::InitializePrototype()
{
    return S_OK;
}

HRESULT VertexIndexBuffer::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT VertexIndexBuffer::BindBuffers()
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVertexBuffer
	};

	_uint			iVertexStrides[] = {
		m_iVertexStride
	};

	_uint			iOffsets[] = {
		0,
	};

	m_pEngineUtility->GetContext()->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pEngineUtility->GetContext()->IASetIndexBuffer(m_pIndexBuffer, m_eIndexFormat, 0);
	m_pEngineUtility->GetContext()->IASetPrimitiveTopology(m_ePrimitive);

	return S_OK;
}

HRESULT VertexIndexBuffer::Render()
{
	m_pEngineUtility->GetContext()->DrawIndexed(m_iNumIndices, 0, 0);
	return S_OK;
}

void VertexIndexBuffer::Free()
{
	__super::Free();

	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pIndexBuffer);
}
