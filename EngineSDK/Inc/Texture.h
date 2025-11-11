#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL Texture final : public Component
{
private:
	Texture();
	Texture(const Texture& Prototype);
	virtual ~Texture() = default;

public:
	virtual HRESULT InitializePrototype(const _tchar* pTextureFilePath, _uint iNumTextures);
	virtual HRESULT Initialize(void* pArg) override;

	HRESULT BindRenderTargetShaderResource(class Shader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT BindShaderResources(class Shader* pShader, const _char* pConstantName);

	static Texture* Create(const _tchar* pTextureFilePath, _uint iNumTextures);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
private:
	_uint											m_iNumTextures = {};
	vector<ID3D11ShaderResourceView*>				m_Textures;
};

NS_END