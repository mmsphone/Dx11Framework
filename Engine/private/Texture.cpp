#include "Texture.h"
#include "EngineUtility.h"
#include "Shader.h"

Texture::Texture()
    :Component{}
{
}

Texture::Texture(const Texture& Prototype)
    :Component{Prototype}
    , m_iNumTextures{ Prototype. m_iNumTextures }
    , m_Textures{ Prototype.m_Textures }
{
    for (auto& pTexture : m_Textures)
        SafeAddRef(pTexture);
}

HRESULT Texture::InitializePrototype(const _tchar* pTextureFilePath, _uint iNumTextures)
{
    m_iNumTextures = iNumTextures;
    m_Textures.reserve(m_iNumTextures);

	for (size_t i = 0; i < iNumTextures; i++)
	{
		_tchar		szTextureFilePath[MAX_PATH] = TEXT("");
		ID3D11ShaderResourceView* pTexture = { nullptr };

		wsprintf(szTextureFilePath, pTextureFilePath, i);

		_tchar		szExt[MAX_PATH] = {};

		_wsplitpath_s(szTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szExt, MAX_PATH);

		if (false == lstrcmp(szExt, TEXT(".dds")))
		{
			if (FAILED(CreateDDSTextureFromFile(m_pEngineUtility->GetDevice(), szTextureFilePath, nullptr, &pTexture)))
				return E_FAIL;
		}
		else if (false == lstrcmp(szExt, TEXT(".tga")))
		{
			return E_FAIL;
		}
		else
		{
			if (FAILED(CreateWICTextureFromFile(m_pEngineUtility->GetDevice(), szTextureFilePath, nullptr, &pTexture)))
				return E_FAIL;
		}

		m_Textures.push_back(pTexture);
	}

	return S_OK;
}

HRESULT Texture::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT Texture::BindRenderTargetShaderResource(Shader* pShader, const _char* pConstantName, _uint iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures) // 없는 텍스처면
		return E_FAIL;

	return pShader->BindRenderTargetShaderResource(pConstantName, m_Textures[iTextureIndex]);
}

HRESULT Texture::BindShaderResources(Shader* pShader, const _char* pConstantName)
{
	return pShader->BindShaderResources(pConstantName, &m_Textures.front(), m_iNumTextures);
}

Texture* Texture::Create(const _tchar* pTextureFilePath, _uint iNumTextures)
{
	Texture* pInstance = new Texture();

	if (FAILED(pInstance->InitializePrototype(pTextureFilePath, iNumTextures)))
	{
		MSG_BOX("Failed to Created : Texture");
		SafeRelease(pInstance);
	}

	return pInstance;
}

Component* Texture::Clone(void* pArg)
{
	Texture* pInstance = new Texture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : Texture");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Texture::Free()
{
	__super::Free();

	for (auto& pTexture : m_Textures)
		SafeRelease(pTexture);

	m_Textures.clear();
}
