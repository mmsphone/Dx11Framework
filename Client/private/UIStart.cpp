#include "UIStart.h"

#include "EngineUtility.h"

UIStart::UIStart()
	:UITemplate{}
{
}

UIStart::UIStart(const UIStart& Prototype)
	:UITemplate{Prototype}
{
}

HRESULT UIStart::Render()
{
	Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
	Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
	Texture* pTexture = dynamic_cast<Texture*>(FindComponent(TEXT("Texture")));
	VIBufferRect* pRect = dynamic_cast<VIBufferRect*>(FindComponent(TEXT("VIBuffer")));

	if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(pTexture->BindShaderResource(pShader, "g_Texture", 0)))
		return E_FAIL;

	pShader->Begin(0);
	pRect->BindBuffers();
	pRect->Render();

	return S_OK;
}

UIStart* UIStart::Create()
{
	UIStart* pInstance = new UIStart();

	if (FAILED(pInstance->InitializePrototype()))
	{
		MSG_BOX("Failed to Created : UIStart");
		SafeRelease(pInstance);
	}

	return pInstance;
}

Object* UIStart::Clone(void* pArg)
{
	UIStart* pInstance = new UIStart(*this);

	UI_DESC uiDesc{};
	uiDesc.fX = 1000;
	uiDesc.fY = 360;
	uiDesc.fSizeX = 200;
	uiDesc.fSizeY = 50;

	if (FAILED(pInstance->Initialize(&uiDesc)))
	{
		MSG_BOX("Failed to Cloned : UIStart");
		SafeRelease(pInstance);
	}

	return pInstance;
}

HRESULT UIStart::ReadyComponents()
{
	/* For.Com_Texture */
	if (FAILED(AddComponent(SCENE::LOGO, TEXT("Texture_UIStart"), TEXT("Texture"), nullptr, nullptr)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(AddComponent(SCENE::LOGO, TEXT("Shader_VtxPosTex"), TEXT("Shader"), nullptr, nullptr)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(AddComponent(SCENE::LOGO, TEXT("VIBuffer_Rect"), TEXT("VIBuffer"), nullptr, nullptr)))
		return E_FAIL;

	return S_OK;
}