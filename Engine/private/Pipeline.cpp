#include "Pipeline.h"

Pipeline::Pipeline()
{
}

const _float4x4* Pipeline::GetTransformFloat4x4Ptr(D3DTS eState)
{
	return &m_TransformationMatrix[ENUM_CLASS(eState)];
}

_matrix Pipeline::GetTransformMatrix(D3DTS eState)
{
	return XMLoadFloat4x4(&m_TransformationMatrix[ENUM_CLASS(eState)]);
}

const _float4x4* Pipeline::GetTransformFloat4x4InversePtr(D3DTS eState)
{
	return &m_TransformationMatrix_Inverse[ENUM_CLASS(eState)];
}

_matrix Pipeline::GetTransformMatrixInverse(D3DTS eState)
{
	return XMLoadFloat4x4(&m_TransformationMatrix_Inverse[ENUM_CLASS(eState)]);
}

const _float4* Pipeline::GetCamPosition()
{
	return &m_vCamPosition;
}

void Pipeline::SetTransform(D3DTS eState, _fmatrix TransformMatrix)
{
	XMStoreFloat4x4(&m_TransformationMatrix[eState], TransformMatrix);
}

HRESULT Pipeline::Initialize()
{
	for (size_t i = 0; i < D3DTS::D3DTS_END; i++)
	{
		XMStoreFloat4x4(&m_TransformationMatrix[i], XMMatrixIdentity());
		XMStoreFloat4x4(&m_TransformationMatrix_Inverse[i], XMMatrixIdentity());
	}

	return S_OK;
}

void Pipeline::Update()
{
	for (size_t i = 0; i < D3DTS::D3DTS_END; i++)
	{
		XMStoreFloat4x4(&m_TransformationMatrix_Inverse[ENUM_CLASS(i)], XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformationMatrix[ENUM_CLASS(i)])));
	}

	memcpy(&m_vCamPosition, &m_TransformationMatrix_Inverse[D3DTS::VIEW].m[3], sizeof(_float4));

}

Pipeline* Pipeline::Create()
{
	Pipeline* pInstance = new Pipeline();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : Pipeline");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Pipeline::Free()
{
	__super::Free();
}
