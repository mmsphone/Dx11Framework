#include "RenderTargetManager.h"

#include "EngineUtility.h"

#include "RenderTarget.h"

RenderTargetManager::RenderTargetManager()
	:m_pEngineUtility{ EngineUtility::GetInstance()}
{
}

HRESULT RenderTargetManager::AddRenderTarget(const _wstring& strRenderTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	if (nullptr != FindRenderTarget(strRenderTargetTag))
		return E_FAIL;

	RenderTarget* pRenderTarget = RenderTarget::Create(iWidth, iHeight, ePixelFormat, vClearColor);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	m_RenderTargets.emplace(strRenderTargetTag, pRenderTarget);

	return S_OK;
}

HRESULT RenderTargetManager::AddRenderTargetGroup(const _wstring& strRenderTargetGroupTag, const _wstring& strRenderTargetTag)
{
	RenderTarget* pRenderTarget = FindRenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	list<RenderTarget*>* pRenderTargetGroupList = FindRenderTargetGroup(strRenderTargetGroupTag);
	if (nullptr == pRenderTargetGroupList)
	{
		list<RenderTarget*>	RenderTargetGroupList;
		RenderTargetGroupList.push_back(pRenderTarget);

		m_RenderTargetGroups.emplace(strRenderTargetGroupTag, RenderTargetGroupList);
	}
	else
		pRenderTargetGroupList->push_back(pRenderTarget);

	SafeAddRef(pRenderTarget);

	return S_OK;
}

HRESULT RenderTargetManager::BeginRenderTargetGroup(const _wstring& strRenderTargetGroupTag, _bool isClearDepth, ID3D11DepthStencilView* pDepthStencilView)
{
	/* strMRTTag로 추가되어있었던 렌더타겟들(list<CRenderTarget*>)을 동시에 장치에 바인딩한다. */
	list<RenderTarget*>* pMRTList = FindRenderTargetGroup(strRenderTargetGroupTag);
	if (nullptr == pMRTList)
		return E_FAIL;

	m_pEngineUtility->GetContext()->OMGetRenderTargets(1, &m_pBackBufferRenderTargetView, &m_pDepthStencilView);

	_uint		iNumRenderTargets = {};

	ID3D11RenderTargetView* pRenderTargets[8] = {};

	/* 동시에 바인딩하는 렌더타겟들은 사이즈가 모두 같아야한다. */
	for (auto& pRenderTarget : *pMRTList)
	{
		pRenderTarget->Clear();
		pRenderTargets[iNumRenderTargets++] = pRenderTarget->GetRenderTargetView();
	}
	if (nullptr != pDepthStencilView &&
		true == isClearDepth)
	{
		m_pEngineUtility->GetContext()->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	/* 깊이버퍼와 렌더타겟들의 사이즈또한 같아야한다. */
	m_pEngineUtility->GetContext()->OMSetRenderTargets(iNumRenderTargets, pRenderTargets, nullptr != pDepthStencilView ? pDepthStencilView : m_pDepthStencilView);

	return S_OK;
}

HRESULT RenderTargetManager::EndRenderTargetGroup()
{
	ID3D11RenderTargetView* pRTVs[8] = { m_pBackBufferRenderTargetView };

	m_pEngineUtility->GetContext()->OMSetRenderTargets(8, pRTVs, m_pDepthStencilView);

	SafeRelease(m_pBackBufferRenderTargetView);
	SafeRelease(m_pDepthStencilView);

	return S_OK;
}

HRESULT RenderTargetManager::BindRenderTargetShaderResource(const _wstring& strRenderTargetTag, Shader* pShader, const _char* pConstantName)
{
	RenderTarget* pRenderTarget = FindRenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->BindRenderTargetShaderResource(pShader, pConstantName);
}

HRESULT RenderTargetManager::CopyRenderTargetResource(const _wstring& strRenderTargetTag, ID3D11Texture2D* pOut)
{
	RenderTarget* pRenderTarget = FindRenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;
	return pRenderTarget->CopyResource(pOut);
}

#ifdef _DEBUG

HRESULT RenderTargetManager::ReadyRenderTargetDebug(const _wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	RenderTarget* pRenderTarget = FindRenderTarget(strRenderTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;
	return pRenderTarget->ReadyRenderTargetDebug(fX, fY, fSizeX, fSizeY);
}

HRESULT RenderTargetManager::RenderRenderTargetGroup(const _wstring& strRenderTargetGroupTag, Shader* pShader, VIBufferRect* pVIBuffer)
{
	list<RenderTarget*>* pRenderTargetGroupList = FindRenderTargetGroup(strRenderTargetGroupTag);
	if (nullptr == pRenderTargetGroupList)
		return E_FAIL;

	for (auto& pRenderTarget : *pRenderTargetGroupList)
	{
		pRenderTarget->Render(pShader, pVIBuffer);
	}

	return S_OK;
}

#endif

RenderTarget* RenderTargetManager::FindRenderTarget(const _wstring& strRenderTargetTag)
{
	auto	iter = m_RenderTargets.find(strRenderTargetTag);
	if (iter == m_RenderTargets.end())
		return nullptr;

	return iter->second;
}

list<RenderTarget*>* RenderTargetManager::FindRenderTargetGroup(const _wstring& strRenderTargetGroupTag)
{
	auto	iter = m_RenderTargetGroups.find(strRenderTargetGroupTag);
	if (iter == m_RenderTargetGroups.end())
		return nullptr;

	return &iter->second;
}

RenderTargetManager* RenderTargetManager::Create()
{
	return new RenderTargetManager();
}

void RenderTargetManager::Free()
{
	__super::Free();

	for (auto& Pair : m_RenderTargetGroups)
	{
		for (auto& pRenderTarget : Pair.second)
			SafeRelease(pRenderTarget);
		Pair.second.clear();
	}
	m_RenderTargetGroups.clear();

	for (auto& Pair : m_RenderTargets)
		SafeRelease(Pair.second);
	m_RenderTargets.clear();

	SafeRelease(m_pEngineUtility);
}
