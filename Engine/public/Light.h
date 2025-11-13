#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL Light final : public Base
{
private:
	Light();
	virtual ~Light() = default;

public:
	HRESULT Initialize(const LIGHT_DESC& lightDesc);
	const LIGHT_DESC* GetLight() const;
	
	HRESULT RenderLight(class Shader* pShader, class VIBufferRect* pVIBuffer);


	static Light* Create(const LIGHT_DESC& lightDesc);
	virtual void Free() override;
private:
	LIGHT_DESC m_LightDesc = {};
};

NS_END