#pragma once

#include "Base.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class Material final : public Base
{
private:
	Material();
	virtual ~Material() = default;

public:
	HRESULT Initialize(const MaterialData& material);
	HRESULT BindRenderTargetShaderResource(class Shader* pShader, const _char* pConstantName, TextureType eType, _uint iIndex);

	static Material* Create(const MaterialData& material);
	virtual void Free() override;

private:
	ID3D11ShaderResourceView* CreateSolidColorSRV(ID3D11Device* dev, _float r, _float g, _float b, _float a);


private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	vector<ID3D11ShaderResourceView*>		m_Textures[TextureType::End];
	_float4 m_vDiffuseColor = { 1.f, 1.f, 1.f, 1.f };
	_float4 m_vSpecularColor = { 0.f, 0.f, 0.f, 1.f };
	_float4 m_vEmissiveColor = { 0.f, 0.f, 0.f, 1.f };
};

NS_END
