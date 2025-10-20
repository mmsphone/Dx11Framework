#include "Shader.h"
#include "EngineUtility.h"

Shader::Shader()
	:Component{}
{
}

Shader::Shader(const Shader& Prototype)
	:Component{ Prototype }
	, m_pEffect { Prototype.m_pEffect }
	, m_iNumPasses { Prototype.m_iNumPasses }
	, m_InputLayouts{ Prototype.m_InputLayouts }
{
	for (auto& pInputLayout : m_InputLayouts)
		SafeAddRef(pInputLayout);

	SafeAddRef(m_pEffect);
}

HRESULT Shader::InitializePrototype(const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	// 셰이더 언어 HLSL 옵션 플래그
	_uint iHlslFlag = {}; 
#ifdef _DEBUG
	iHlslFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	iHlslFlag = D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

	ID3DBlob* pErrorMsg = {}; // 에러 데이터
	/* (char*)pErrorMsg->GetBufferPointer(); 으로 에러 데이터 꺼내볼 수 있음*/

	//ID3DX11Effect 생성
	if (FAILED(D3DX11CompileEffectFromFile(pShaderFilePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pEngineUtility->GetDevice(), &m_pEffect, &pErrorMsg)))
	{
		if (pErrorMsg)
		{
			const char* errorMsg = (const char*)pErrorMsg->GetBufferPointer();
			OutputDebugStringA(errorMsg); // 디버그 창에 출력
			MessageBoxA(nullptr, errorMsg, "Shader Compile Error", MB_OK); // 팝업창으로도 확인 가능
		}
		return E_FAIL;
	}
	//셰이더 정보
	D3DX11_TECHNIQUE_DESC	TechniqueDesc{};

	ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechniqueByIndex(0);
	pTechnique->GetDesc(&TechniqueDesc);

	m_iNumPasses = TechniqueDesc.Passes;

	for (_uint i = 0; i < m_iNumPasses; i++)
	{
		ID3D11InputLayout* pInputLayout = { nullptr };

		ID3DX11EffectPass* pPass = pTechnique->GetPassByIndex(i);

		D3DX11_PASS_DESC		PassDesc{};

		pPass->GetDesc(&PassDesc);

		if (FAILED(m_pEngineUtility->GetDevice()->CreateInputLayout(pElements, iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
			return E_FAIL;

		m_InputLayouts.push_back(pInputLayout);
	}

	return S_OK;
}

HRESULT Shader::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT Shader::Begin(_uint iPassIndex)
{
	if (iPassIndex >= m_iNumPasses) // 사용할 패스가 없으면
		return E_FAIL;

	// Context에 InputLayout 바인딩
	m_pEngineUtility->GetContext()->IASetInputLayout(m_InputLayouts[iPassIndex]);

	// 셰이더 패스 적용
	m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, m_pEngineUtility->GetContext());
	return S_OK;
}

HRESULT Shader::BindRawValue(const _char* pConstantName, const void* pData, _uint iSize)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	return pVariable->SetRawValue(pData, 0, iSize);
}

HRESULT Shader::BindMatrix(const _char* pConstantName, const _float4x4* pMatrix)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrix(reinterpret_cast<const _float*>(pMatrix));
}

HRESULT Shader::BindMatrices(const _char* pConstantName, const _float4x4* pMatrix, _uint iNumMatrices)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrixArray(reinterpret_cast<const _float*>(pMatrix), 0, iNumMatrices);
}

HRESULT Shader::BindShaderResource(const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResource(pSRV);

}

HRESULT Shader::BindShaderResources(const _char* pConstantName, ID3D11ShaderResourceView** ppSRVs, _uint iNumSRVs)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResourceArray(ppSRVs, 0, iNumSRVs);

}

Shader* Shader::Create(const _tchar* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	Shader* pInstance = new Shader();

	if (FAILED(pInstance->InitializePrototype(pShaderFilePath, pElements, iNumElements)))
	{
		MSG_BOX("Failed to Created : Shader");
		SafeRelease(pInstance);
	}

	return pInstance;
}

Component* Shader::Clone(void* pArg)
{
	Shader* pInstance = new Shader(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : Shader");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Shader::Free()
{
	__super::Free();

	for (auto& pInputLayout : m_InputLayouts)
		SafeRelease(pInputLayout);

	m_InputLayouts.clear();

	SafeRelease(m_pEffect);
}
