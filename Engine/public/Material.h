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
	HRESULT BindShaderResource(class Shader* pShader, const _char* pConstantName, TextureType eType, _uint iIndex);

	static Material* Create(const MaterialData& material);
	virtual void Free() override;
private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	vector<ID3D11ShaderResourceView*>		m_Textures[TextureType::End];
};

NS_END
