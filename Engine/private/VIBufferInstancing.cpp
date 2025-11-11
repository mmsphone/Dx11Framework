#include "VIBufferInstancing.h"

#include "EngineUtility.h"

VIBufferInstancing::VIBufferInstancing()
	: VertexIndexBuffer{  }
{
}

VIBufferInstancing::VIBufferInstancing(const VIBufferInstancing& Prototype)
	: VertexIndexBuffer{ Prototype }
	//, m_pVBInstance { Prototype.m_pVBInstance }
	, m_iInstanceVertexStride{ Prototype.m_iInstanceVertexStride }
	, m_iNumInstance{ Prototype.m_iNumInstance }
	, m_iIndexCountPerInstance{ Prototype.m_iIndexCountPerInstance }
	, m_InstanceDesc{ Prototype.m_InstanceDesc }
{
}

HRESULT VIBufferInstancing::InitializePrototype(const INSTANCE_DESC* pDesc)
{
	return S_OK;
}

HRESULT VIBufferInstancing::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT VIBufferInstancing::BindBuffers()
{

	ID3D11Buffer* pVertexBuffers[] = {
		m_pVertexBuffer,
		m_pVBInstance,
	};

	_uint			iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceVertexStride,

	};

	_uint			iOffsets[] = {
		0,
		0
	};

	m_pEngineUtility->GetContext()->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pEngineUtility->GetContext()->IASetIndexBuffer(m_pIndexBuffer, m_eIndexFormat, 0);
	m_pEngineUtility->GetContext()->IASetPrimitiveTopology(m_ePrimitive);

	return S_OK;
}

HRESULT VIBufferInstancing::Render()
{
	m_pEngineUtility->GetContext()->DrawIndexedInstanced(m_iIndexCountPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

void VIBufferInstancing::Free()
{
	__super::Free();

	SafeRelease(m_pVBInstance);
}
