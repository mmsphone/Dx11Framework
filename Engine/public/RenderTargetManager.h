#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class RenderTargetManager final : public Base
{
private:
	RenderTargetManager();
	virtual ~RenderTargetManager() = default;

public:
	HRESULT AddRenderTarget(const _wstring& strRenderTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT AddRenderTargetGroup(const _wstring& strRenderTargetGroupTag, const _wstring& strRenderTargetTag);
	HRESULT BeginRenderTargetGroup(const _wstring& strRenderTargetGroupTag, _bool isClearDepth, ID3D11DepthStencilView* pDepthStencilView);
	HRESULT EndRenderTargetGroup();
	HRESULT BindRenderTargetShaderResource(const _wstring& strRenderTargetTag, class Shader* pShader, const _char* pConstantName);
	HRESULT CopyRenderTargetResource(const _wstring& strRenderTargetTag, ID3D11Texture2D* pOut);
#ifdef _DEBUG
	HRESULT ReadyRenderTargetDebug(const _wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT RenderRenderTargetGroup(const _wstring& strRenderTargetGroupTag, class Shader* pShader, class VIBufferRect* pVIBuffer);
#endif

	static RenderTargetManager* Create();
	virtual void Free() override;

private:
	class RenderTarget* FindRenderTarget(const _wstring& strRenderTargetTag);
	list<class RenderTarget*>* FindRenderTargetGroup(const _wstring& strRenderTargetGroupTag);

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	unordered_map<_wstring, class RenderTarget*>			m_RenderTargets;
	unordered_map<_wstring, list<class RenderTarget*>>		m_RenderTargetGroups;

	ID3D11RenderTargetView* m_pBackBufferRenderTargetView = { nullptr };
	ID3D11DepthStencilView* m_pDepthStencilView = { nullptr };

};

NS_END