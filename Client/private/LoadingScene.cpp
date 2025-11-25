#include "LoadingScene.h"

#include "EngineUtility.h"
#include "Loader.h"

#include "LogoScene.h"
#include "GameScene.h"

#include "UIImage.h"
#include "Layer.h"

LoadingScene::LoadingScene()
	: Scene{}
{
}


HRESULT LoadingScene::Initialize(SCENE eNextSceneId)
{
	m_eNextSceneId = eNextSceneId;

	if (FAILED(ReadyLayerBackGround()))
		return E_FAIL;

	m_pLoader = Loader::Create(eNextSceneId);
	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void LoadingScene::Update(_float fTimeDelta)
{
 	if (true == m_pLoader->isFinished())
	{
		Scene* pNextScene = { nullptr };

		switch (m_eNextSceneId)
		{
		case SCENE::LOGO:
			pNextScene = LogoScene::Create();
			break;
		case SCENE::GAMEPLAY:
			pNextScene = GameScene::Create();
			break;
		}
		if (FAILED(m_pEngineUtility->ChangeScene(m_eNextSceneId, pNextScene)))
			return;

		return;
	}
}

HRESULT LoadingScene::Render()
{
	return S_OK;
}

HRESULT LoadingScene::ReadyLayerBackGround()
{
	switch (m_eNextSceneId)
	{
	case SCENE::LOGO:
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Loading_Logo";
		desc.x = g_iWinSizeX * 0.5f;
		desc.y = g_iWinSizeY * 0.5f;
		desc.z = 0.f;
		desc.w = 1280.f;
		desc.h = 720.f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Loading/Loading_Logo.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::LOADING, L"UI", &desc);
	}
		break;
	case SCENE::GAMEPLAY:
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Loading_GameScene";
		desc.x = g_iWinSizeX * 0.5f;
		desc.y = g_iWinSizeY * 0.5f;
		desc.z = 0.f;
		desc.w = 1280.f;
		desc.h = 720.f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Loading/Loading_GameScene.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::LOADING, L"UI", &desc);
	}
		break;
	}
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Loadingbar_Back";
		desc.x = 920.f;
		desc.y = 650.f;
		desc.z = 0.f;
		desc.w = 256.f * 2.5f;
		desc.h = 64.f * 2.f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Loading/Loadingbar_Back.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::LOADING, L"UI", &desc);
	}
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Loadingbar_Front";
		desc.x = 920.f;
		desc.y = 650.f;
		desc.z = 0.f;
		desc.w = 256.f * 2.5f;
		desc.h = 64.f * 2.f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Loading/Loadingbar_Front.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::LOADING, L"UI", &desc);

		UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindLayer(SCENE::LOADING, L"UI")->GetAllObjects().back());
		pUI->SetLoadingRatio(0.f);
		m_pEngineUtility->AddUI(L"Loadingbar_Front", pUI);
	}

	return S_OK;
}

LoadingScene* LoadingScene::Create(SCENE eNextLevelID)
{
	LoadingScene* pInstance = new LoadingScene();

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX("Failed to Created : LoadingScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void LoadingScene::Free()
{
	__super::Free();

	SafeRelease(m_pLoader);
}
