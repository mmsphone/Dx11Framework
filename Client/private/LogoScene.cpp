#include "LogoScene.h"

#include "EngineUtility.h"
#include "LoadingScene.h"

LogoScene::LogoScene()
	: Scene{ }
{
}

HRESULT LogoScene::Initialize()
{
	if (FAILED(ReadyLayerBackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	return S_OK;
}

void LogoScene::Update(_float fTimeDelta)
{
	if (GetKeyState(VK_RETURN) & 0x8000)
	{
		if (FAILED(m_pEngineUtility->ChangeScene(SCENE::LOADING, LoadingScene::Create(SCENE::GAMEPLAY))))
			return;

		return;
	}
}

HRESULT LogoScene::Render()
{
	SetWindowText(g_hWnd, TEXT("로고씬입니다."));

	return S_OK;
}

HRESULT LogoScene::ReadyLayerBackGround(const _tchar* pLayerTag)
{
	if (FAILED(m_pEngineUtility->AddObject(SCENE::LOGO, TEXT("Prototype_GameObject_BackGround"), SCENE::LOGO, pLayerTag)))
		return E_FAIL;

	return S_OK;
}

LogoScene* LogoScene::Create()
{
	LogoScene* pInstance = new LogoScene();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : LogoScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void LogoScene::Free()
{
	__super::Free();
}
