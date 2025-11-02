#include "Material.h"

#include "EngineUtility.h"
#include "Shader.h"

Material::Material()
	:m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT Material::Initialize(const MaterialData& material)
{
	m_vDiffuseColor = material.diffuseColor;
	m_vSpecularColor = material.specularColor;
	m_vEmissiveColor = material.emissiveColor;

	for (int i = 0; i < (int)TextureType::End; ++i)
	{
		for (auto& texPath : material.texturePaths[i])
		{
			_tchar szWFullPath[MAX_PATH] = {};
			MultiByteToWideChar(CP_ACP, 0, texPath.c_str(), -1, szWFullPath, MAX_PATH);

			ID3D11ShaderResourceView* pSRV = nullptr;
			HRESULT hr = E_FAIL;

			// 확장자에 따라 로딩 방식 결정
			std::string ext = texPath.substr(texPath.find_last_of("."));
			if (_stricmp(ext.c_str(), ".dds") == 0)
				hr = DirectX::CreateDDSTextureFromFile(m_pEngineUtility->GetDevice(), szWFullPath, nullptr, &pSRV);
			else
				hr = DirectX::CreateWICTextureFromFile(m_pEngineUtility->GetDevice(), szWFullPath, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures[i].push_back(pSRV);
		}
	}

	return S_OK;
}

HRESULT Material::BindShaderResource(Shader* pShader, const _char* pConstantName, TextureType eType, _uint iIndex)
{
	if (iIndex < m_Textures[(int)eType].size() && m_Textures[(int)eType][iIndex])
	{
		_int bUseTex = 1;
		pShader->BindRawValue("g_bUseTexture", &bUseTex, sizeof(_int));
		return pShader->BindShaderResource(pConstantName, m_Textures[(_int)eType][iIndex]);
	}

	_int bUseTex = 0;
	pShader->BindRawValue("g_bUseTexture", &bUseTex, sizeof(_int));
	pShader->BindRawValue("g_vDiffuseColor", &m_vDiffuseColor, sizeof(_float4));

	return S_OK;	
}

Material* Material::Create(const MaterialData& material)
{
	Material* pInstance = new Material();

	if (FAILED(pInstance->Initialize(material)))
	{
		MSG_BOX("Failed to Created : Material");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Material::Free()
{
	__super::Free();

	for (auto& textures : m_Textures)
	{
		for (auto& pSRV : textures)
			SafeRelease(pSRV);
		textures.clear();
	}
		
	SafeRelease(m_pEngineUtility);
}
