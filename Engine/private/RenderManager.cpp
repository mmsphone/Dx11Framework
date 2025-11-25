#include "RenderManager.h"

#include "EngineUtility.h"

#include "Object.h"

RenderManager::RenderManager()
    :m_pEngineUtility{ EngineUtility::GetInstance() }
{
    SafeAddRef(m_pEngineUtility);
}

HRESULT RenderManager::Initialize()
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pEngineUtility->GetContext()->RSGetViewports(&iNumViewports, &ViewportDesc);

	/* Target_Diffuse */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_Diffuse"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* Target_Normal */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_Normal"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* Target_Depth */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_Depth"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* Target_Shade */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_Shade"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* Target_Specular */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_Specular"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* Target_Shadow */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_ShadowLight1"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_ShadowLight2"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_ShadowLight3"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_ShadowLight4"), g_iMaxWidth, g_iMaxHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.0f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	/* Target_World */
	if (FAILED(m_pEngineUtility->AddRenderTarget(TEXT("RenderTarget_World"), static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height), DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(ReadyShadowDepthStencilView()))
		return E_FAIL;

	/* MRT_GameObjects : 이 타겟들은 게임오브젝트의 정보를 저장받기위한 타겟들이다.  */
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_Objects"), TEXT("RenderTarget_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_Objects"), TEXT("RenderTarget_Normal"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_Objects"), TEXT("RenderTarget_Depth"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_Objects"), TEXT("RenderTarget_World"))))
		return E_FAIL;

	/* MRT_LightAcc : 이 타겟들은 빛들의 연산결과를 누적저장받기위한 타겟들이다. */
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_LightAcc"), TEXT("RenderTarget_Shade"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_LightAcc"), TEXT("RenderTarget_Specular"))))
		return E_FAIL;

	/* MRT_Shadow */
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_ShadowLight"), TEXT("RenderTarget_ShadowLight1"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_ShadowLight"), TEXT("RenderTarget_ShadowLight2"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_ShadowLight"), TEXT("RenderTarget_ShadowLight3"))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddRenderTargetGroup(TEXT("RenderTargetGroup_ShadowLight"), TEXT("RenderTarget_ShadowLight4"))))
		return E_FAIL;


	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f));

	m_pShader = Shader::Create(TEXT("../bin/Shader/Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pVIBuffer = VIBufferRect::Create();
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_Diffuse"), 100.0f, 100.0f, 200.f, 200.f)))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_Normal"), 100.0f, 300.0f, 200.f, 200.f)))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_Depth"), 100.0f, 500.0f, 200.f, 200.f)))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_Shade"), 350.0f, 150.0f, 300.f, 300.f)))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_Specular"), 350.0f, 450.0f, 300.f, 300.f)))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_ShadowLight1"), ViewportDesc.Width - 150.0f, 150.0f, 300.f, 300.f)))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->ReadyRenderTargetDebug(TEXT("RenderTarget_ShadowLight2"), ViewportDesc.Width - 150.0f, 450.0f, 300.f, 300.f)))
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT RenderManager::JoinRenderGroup(RENDERGROUP eGroupId, Object* pObject)
{
    m_RenderObjects[eGroupId].push_back(pObject);
    SafeAddRef(pObject);

    return S_OK;
}

void RenderManager::Draw()
{
    RenderPriority(); 
	RenderShadowLight();
    RenderNonBlend();
	RenderLights();
	RenderCombined();
	RenderNonLights();
    RenderBlend();
    m_pEngineUtility->SnapDepthForPicking();
	RenderUI();
	renderMouse();
}

#ifdef _DEBUG
void RenderManager::RenderDebug()
{
	for (auto& pDebugComponent : m_DebugComponents)
	{
		pDebugComponent->Render();
		SafeRelease(pDebugComponent);
	}
	m_DebugComponents.clear();

	if (FAILED(m_pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
		return;

	if (FAILED(m_pVIBuffer->BindBuffers()))
		return;

	if (FAILED(m_pEngineUtility->RenderDebugRenderTargetGroup(TEXT("RenderTargetGroup_Objects"), m_pShader, m_pVIBuffer)))
		return;
	if (FAILED(m_pEngineUtility->RenderDebugRenderTargetGroup(TEXT("RenderTargetGroup_LightAcc"), m_pShader, m_pVIBuffer)))
		return;
	if (FAILED(m_pEngineUtility->RenderDebugRenderTargetGroup(TEXT("RenderTargetGroup_ShadowLight"), m_pShader, m_pVIBuffer)))
		return;
}
HRESULT RenderManager::AddDebugComponent(Component* pDebugComponent)
{
	m_DebugComponents.push_back(pDebugComponent);
	SafeAddRef(pDebugComponent);
	return S_OK;
}
#endif

RenderManager* RenderManager::Create()
{
    RenderManager* pInstance = new RenderManager();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : RenderManager");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void RenderManager::Free()
{
    __super::Free();
    
	for (_uint i = RENDERGROUP::RENDER_PRIORITY; i < RENDERGROUP::RENDERGROUP_END; ++i)
	{
		for (auto& pObject : m_RenderObjects[i])
		{
			SafeRelease(pObject);
		}
	}

	SafeRelease(m_pShader);
	SafeRelease(m_pVIBuffer);

	SafeRelease(m_pShadowDepthStencilView);

	SafeRelease(m_pEngineUtility);
}

void RenderManager::RenderPriority()
{
    for (auto& pObject : m_RenderObjects[RENDERGROUP::RENDER_PRIORITY])
    {
        if(pObject!= nullptr && pObject->IsDead() == false)
            pObject->Render();

        SafeRelease(pObject);
    }
    m_RenderObjects[RENDERGROUP::RENDER_PRIORITY].clear();
}

void RenderManager::RenderShadowLight()
{
 	if (FAILED(m_pEngineUtility->BeginRenderTargetGroup(TEXT("RenderTargetGroup_ShadowLight"), true, m_pShadowDepthStencilView)))
		return;

	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pEngineUtility->GetContext()->RSGetViewports(&iNumViewports, &ViewportDesc);

	ChangeViewportSize(g_iMaxWidth, g_iMaxHeight);
	_uint iNumShadowLights = m_pEngineUtility->GetNumActiveShadowLights();
	for (auto& pRenderObject : m_RenderObjects[RENDER_SHADOWLIGHT])
	{
		if (nullptr != pRenderObject && pRenderObject->IsDead() == false)
		{
			for(_uint i = 0; i< iNumShadowLights; ++i)
				pRenderObject->RenderShadow(i);
		}

		SafeRelease(pRenderObject);
	}
	m_RenderObjects[RENDER_SHADOWLIGHT].clear();

	if (FAILED(m_pEngineUtility->EndRenderTargetGroup()))
		return;

	ChangeViewportSize(static_cast<_uint>(ViewportDesc.Width), static_cast<_uint>(ViewportDesc.Height));
}

void RenderManager::RenderNonBlend()
{
	if (FAILED(m_pEngineUtility->BeginRenderTargetGroup(TEXT("RenderTargetGroup_Objects"))))
		return;

    for (auto& pObject : m_RenderObjects[RENDER_NONBLEND])
    {
        if (pObject != nullptr && pObject->IsDead() == false)
            pObject->Render();

        SafeRelease(pObject);
    }
    m_RenderObjects[RENDER_NONBLEND].clear();

	if (FAILED(m_pEngineUtility->EndRenderTargetGroup()))
		return;
}

void RenderManager::RenderLights()
{
	if (FAILED(m_pEngineUtility->BeginRenderTargetGroup(TEXT("RenderTargetGroup_LightAcc"))))
		return;

	if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_Normal"), m_pShader, "g_NormalTexture")))
		return;
	if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_Depth"), m_pShader, "g_DepthTexture")))
		return;

	if (FAILED(m_pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
		return;
	if (FAILED(m_pShader->BindRawValue("g_fCamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
		return;
	if (FAILED(m_pShader->BindMatrix("g_WorldMatrix", &m_WorldMatrix)))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ViewMatrixInverse", m_pEngineUtility->GetTransformFloat4x4InversePtr(D3DTS_VIEW))))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ProjMatrixInverse", m_pEngineUtility->GetTransformFloat4x4InversePtr(D3DTS_PROJECTION))))
		return;


	if (FAILED(m_pEngineUtility->RenderLights(m_pShader, m_pVIBuffer)))
		return;

	if (FAILED(m_pEngineUtility->EndRenderTargetGroup()))
		return;
}

void RenderManager::RenderCombined()
{
	if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return;
	if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_Shade"), m_pShader, "g_ShadeTexture")))
		return;
	if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_Specular"), m_pShader, "g_SpecularTexture")))
		return;
	if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_Depth"), m_pShader, "g_DepthTexture")))
		return;

	if (FAILED(m_pShader->BindMatrix("g_ViewMatrixInverse", m_pEngineUtility->GetTransformFloat4x4InversePtr(D3DTS_VIEW))))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ProjMatrixInverse", m_pEngineUtility->GetTransformFloat4x4InversePtr(D3DTS_PROJECTION))))
		return;
	if (FAILED(m_pShader->BindMatrix("g_WorldMatrix", &m_WorldMatrix)))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
		return;
	if (FAILED(m_pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
		return;
	if (FAILED(m_pShader->BindRawValue("g_fCamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
		return;

	_uint iNumShadowLights = m_pEngineUtility->GetNumActiveShadowLights();
	if (FAILED(m_pShader->BindRawValue("g_NumShadowLights", &iNumShadowLights, sizeof(_uint))))
		return;

	if (iNumShadowLights > 0)
	{
		if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_ShadowLight1"), m_pShader, "g_ShadowTexture1")))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightViewMatrix1", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_VIEW, 0))))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightProjMatrix1", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_PROJECTION, 0))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightPosition1", m_pEngineUtility->GetActiveShadowLightPositionPtr(0), sizeof(_float3))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightFarDistance1", m_pEngineUtility->GetActiveShadowLightFarDistancePtr(0), sizeof(_float))))
			return;
	}
	if (iNumShadowLights > 1)
	{
		if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_ShadowLight2"), m_pShader, "g_ShadowTexture2")))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightViewMatrix2", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_VIEW, 1))))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightProjMatrix2", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_PROJECTION, 1))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightPosition2", m_pEngineUtility->GetActiveShadowLightPositionPtr(1), sizeof(_float3))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightFarDistance2", m_pEngineUtility->GetActiveShadowLightFarDistancePtr(1), sizeof(_float))))
			return;
	}
	if (iNumShadowLights > 2)
	{
		if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_ShadowLight3"), m_pShader, "g_ShadowTexture3")))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightViewMatrix3", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_VIEW, 2))))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightProjMatrix3", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_PROJECTION, 2))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightPosition3", m_pEngineUtility->GetActiveShadowLightPositionPtr(2), sizeof(_float3))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightFarDistance3", m_pEngineUtility->GetActiveShadowLightFarDistancePtr(2), sizeof(_float))))
			return;
	}
	if (iNumShadowLights > 3)
	{
		if (FAILED(m_pEngineUtility->BindRenderTargetShaderResource(TEXT("RenderTarget_ShadowLight4"), m_pShader, "g_ShadowTexture4")))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightViewMatrix4", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_VIEW, 3))))
			return;
		if (FAILED(m_pShader->BindMatrix("g_ShadowLightProjMatrix4", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS_PROJECTION, 3))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightPosition4", m_pEngineUtility->GetActiveShadowLightPositionPtr(3), sizeof(_float3))))
			return;
		if (FAILED(m_pShader->BindRawValue("g_ShadowLightFarDistance4", m_pEngineUtility->GetActiveShadowLightFarDistancePtr(3), sizeof(_float))))
			return;
	}

	m_pShader->Begin(3);
	m_pVIBuffer->BindBuffers();
	m_pVIBuffer->Render();
}

void RenderManager::RenderNonLights()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_NONLIGHT])
	{
		if (nullptr != pRenderObject && pRenderObject->IsDead() == false)
			pRenderObject->Render();

		SafeRelease(pRenderObject);
	}
	m_RenderObjects[RENDER_NONLIGHT].clear();


}

void RenderManager::RenderBlend()
{
    for (auto& pObject : m_RenderObjects[RENDER_BLEND])
    {
        if (pObject != nullptr && pObject->IsDead() == false)
            pObject->Render();
        SafeRelease(pObject);
    }
    m_RenderObjects[RENDER_BLEND].clear();
}

void RenderManager::RenderUI()
{
    for (auto& pObject : m_RenderObjects[RENDER_UI])
    {
        if (pObject != nullptr && pObject->IsDead() == false)
            pObject->Render();
        SafeRelease(pObject);
    }
    m_RenderObjects[RENDER_UI].clear();
}

void RenderManager::renderMouse()
{
	for (auto& pObject : m_RenderObjects[RENDER_MOUSE])
	{
		if (pObject != nullptr && pObject->IsDead() == false)
			pObject->Render();
		SafeRelease(pObject);
	}
	m_RenderObjects[RENDER_MOUSE].clear();
}

HRESULT RenderManager::ReadyShadowDepthStencilView()
{
	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	/* 깊이 버퍼의 픽셀은 백버퍼의 픽셀과 갯수가 동일해야만 깊이 텍스트가 가능해진다. */
	/* 픽셀의 수가 다르면 아에 렌더링을 못함. */
	TextureDesc.Width = g_iMaxWidth;
	TextureDesc.Height = g_iMaxHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	/* 동적? 정적?  */
	TextureDesc.Usage = D3D11_USAGE_DEFAULT /* 정적 */;
	/* 추후에 어떤 용도로 바인딩 될 수 있는 View타입의 텍스쳐를 만들기위한 Texture2D입니까? */
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL
		/*| D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE*/;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pEngineUtility->GetDevice()->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;

	/* RenderTargetView */
	/* ShaderResourceView */
	/* DepthStencilView */

	if (FAILED(m_pEngineUtility->GetDevice()->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pShadowDepthStencilView)))
		return E_FAIL;

	SafeRelease(pDepthStencilTexture);

	return S_OK;
}

HRESULT RenderManager::ChangeViewportSize(_uint iWidth, _uint iHeight)
{
	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = (_float)iWidth;
	ViewPortDesc.Height = (_float)iHeight;
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pEngineUtility->GetContext()->RSSetViewports(1, &ViewPortDesc);
		
	return S_OK;
}
