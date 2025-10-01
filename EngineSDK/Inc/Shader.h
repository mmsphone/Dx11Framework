#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL Shader final : public Component
{
private:
	Shader();
	Shader(const Shader& Prototype);
	virtual ~Shader() = default;

public:
	virtual HRESULT InitializePrototype(const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Begin(_uint iPassIndex);
	HRESULT BindRawValue(const _char* pConstantName, const void* pData, _uint iSize);
	HRESULT BindMatrix(const _char* pConstantName, const _float4x4* pMatrix);
	HRESULT BindMatrices(const _char* pConstantName, const _float4x4* pMatrix, _uint iNumMatrices);
	HRESULT BindShaderResource(const _char* pConstantName, ID3D11ShaderResourceView* pSRV);
	HRESULT BindShaderResources(const _char* pConstantName, ID3D11ShaderResourceView** ppSRVs, _uint iNumSRVs);

	static Shader* Create(const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
private:
	ID3DX11Effect* m_pEffect = { nullptr };
	_uint						m_iNumPasses = {};
	vector<ID3D11InputLayout*>	m_InputLayouts = {};
};

NS_END