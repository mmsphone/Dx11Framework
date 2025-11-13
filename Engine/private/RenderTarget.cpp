#include "RenderTarget.h"

#include "EngineUtility.h"

RenderTarget::RenderTarget()
	:m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT RenderTarget::Initialize(_uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	m_vClearColor = vClearColor;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	/* 깊이 버퍼의 픽셀은 백버퍼의 픽셀과 갯수가 동일해야만 깊이 텍스트가 가능해진다. */
	/* 픽셀의 수가 다르면 아에 렌더링을 못함. */
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = ePixelFormat;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;

	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;


	if (FAILED(m_pEngineUtility->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	if (FAILED(m_pEngineUtility->GetDevice()->CreateRenderTargetView(m_pTexture2D, nullptr, &m_pRenderTargetView)))
		return E_FAIL;

	if (FAILED(m_pEngineUtility->GetDevice()->CreateShaderResourceView(m_pTexture2D, nullptr, &m_pShaderResourceView)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
HRESULT RenderTarget::ReadyRenderTargetDebug(_float fX, _float fY, _float fSizeX, _float fSizeY)
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pEngineUtility->GetContext()->RSGetViewports(&iNumViewports, &ViewportDesc);

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(fSizeX, fSizeY, 1.f) * XMMatrixTranslation(
		fX - ViewportDesc.Width * 0.5f,
		-fY + ViewportDesc.Height * 0.5f,
		0.0f));

	return S_OK;
}

HRESULT RenderTarget::RenderDebug(Shader* pShader, VIBufferRect* pVIBuffer)
{
	pShader->BindMatrix("g_WorldMatrix", &m_WorldMatrix);

	pShader->BindRenderTargetShaderResource("g_Texture", m_pShaderResourceView);

	pShader->Begin(0);

	pVIBuffer->Render();

	return S_OK;
}
#endif

HRESULT RenderTarget::BindRenderTargetShaderResource(Shader* pShader, const _char* pConstantName)
{
	return pShader->BindRenderTargetShaderResource(pConstantName, m_pShaderResourceView);
}

HRESULT RenderTarget::CopyResource(ID3D11Texture2D* pOut)
{
	m_pEngineUtility->GetContext()->CopyResource(pOut, m_pTexture2D);
	return S_OK;
}

ID3D11RenderTargetView* RenderTarget::GetRenderTargetView() const
{
	return m_pRenderTargetView;
}

void RenderTarget::Clear()
{
	m_pEngineUtility->GetContext()->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<_float*>(&m_vClearColor));
}

RenderTarget* RenderTarget::Create(_uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	RenderTarget* pInstance = new RenderTarget();

	if (FAILED(pInstance->Initialize(iWidth, iHeight, ePixelFormat, vClearColor)))
	{
		MSG_BOX("Failed to Created : RenderTarget");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void RenderTarget::Free()
{
	SafeRelease(m_pEngineUtility);
	SafeRelease(m_pShaderResourceView);
	SafeRelease(m_pRenderTargetView);
	SafeRelease(m_pTexture2D);
}
