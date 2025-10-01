#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class Pipeline final : public Base
{
private:
	Pipeline();
	virtual ~Pipeline() = default;

public:
	HRESULT Initialize();
	void Update();

	const _float4x4* GetTransformFloat4x4Ptr(D3DTS eState);
	_matrix GetTransformMatrix(D3DTS eState);
	const _float4x4* GetTransformFloat4x4InversePtr(D3DTS eState);
	_matrix GetTransformMatrixInverse(D3DTS eState);

	const _float4* GetCamPosition();
	void SetTransform(D3DTS eState, _fmatrix TransformMatrix);

	static Pipeline* Create();
	virtual void Free();

private:
	_float4x4				 m_TransformationMatrix[D3DTS::D3DTS_END] = {};
	_float4x4				 m_TransformationMatrix_Inverse[D3DTS::D3DTS_END] = {};
	_float4					 m_vCamPosition = { 0.0f, 0.f, 0.f, 1.f };
};

NS_END