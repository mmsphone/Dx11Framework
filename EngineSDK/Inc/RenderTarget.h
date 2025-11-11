#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class RenderTarget final : public Base
{
	RenderTarget();
	virtual ~RenderTarget() = default;

public:
	HRESULT Initialize(_uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
#ifdef _DEBUG
	HRESULT ReadyRenderTargetDebug(_float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render(class Shader* pShader, class VIBufferRect* pVIBuffer);
#endif
	HRESULT BindRenderTargetShaderResource(class Shader* pShader, const _char* pConstantName);
	HRESULT CopyResource(ID3D11Texture2D* pOut);
	ID3D11RenderTargetView* GetRenderTargetView() const;
	void Clear();

	static RenderTarget* Create(_uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	virtual void Free() override;

private:
	class EngineUtility*		m_pEngineUtility = { nullptr };
	ID3D11Texture2D*			m_pTexture2D = { nullptr };
	ID3D11RenderTargetView*		m_pRenderTargetView = { nullptr };
	ID3D11ShaderResourceView*	m_pShaderResourceView = { nullptr };
	_float4						m_vClearColor = {};

#ifdef _DEBUG
	_float4x4					m_WorldMatrix = {};
#endif
};

NS_END