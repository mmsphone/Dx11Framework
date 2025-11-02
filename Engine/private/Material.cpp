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
	_bool loaded = false;
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
			{
				loaded = true;
				hr = DirectX::CreateDDSTextureFromFile(m_pEngineUtility->GetDevice(), szWFullPath, nullptr, &pSRV);
			}
			else if (_stricmp(ext.c_str(), ".fbm") == 0)
			{
				continue;
			}
			else
			{
				loaded = true;
				hr = DirectX::CreateWICTextureFromFile(m_pEngineUtility->GetDevice(), szWFullPath, nullptr, &pSRV);
			}
			if (FAILED(hr))
				return E_FAIL;

			m_Textures[i].push_back(pSRV);
		}
		if (loaded == false)
		{
			ID3D11ShaderResourceView* srv = CreateSolidColorSRV(m_pEngineUtility->GetDevice(),
				m_vDiffuseColor.x, m_vDiffuseColor.y, m_vDiffuseColor.z, m_vDiffuseColor.w);
			if (srv)
				m_Textures[i].push_back(srv);
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

ID3D11ShaderResourceView* Material::CreateSolidColorSRV(ID3D11Device* dev, _float r, _float g, _float b, _float a)
{
	D3D11_TEXTURE2D_DESC td{};
	td.Width = td.Height = 1;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_IMMUTABLE;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	const uint8_t R = (uint8_t)std::clamp(r * 255.f, 0.f, 255.f);
	const uint8_t G = (uint8_t)std::clamp(g * 255.f, 0.f, 255.f);
	const uint8_t B = (uint8_t)std::clamp(b * 255.f, 0.f, 255.f);
	const uint8_t A = (uint8_t)std::clamp(a * 255.f, 0.f, 255.f);
	uint32_t pixel = (A << 24) | (B << 16) | (G << 8) | (R);

	D3D11_SUBRESOURCE_DATA init{};
	init.pSysMem = &pixel;
	init.SysMemPitch = 4;

	ID3D11Texture2D* tex = nullptr;
	if (FAILED(dev->CreateTexture2D(&td, &init, &tex))) return nullptr;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.Format = td.Format;
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView* srv = nullptr;
	if (SUCCEEDED(dev->CreateShaderResourceView(tex, &sd, &srv))) {
		tex->Release();
		return srv;
	}
	tex->Release();
	return nullptr;
}
