#include "LoadingScene.h"

#include "EngineUtility.h"
#include "Loader.h"

#include "LogoScene.h"
#include "GameScene.h"

LoadingScene::LoadingScene()
	: Scene{}
{
}


HRESULT LoadingScene::Initialize(SCENE eNextSceneId)
{
	m_eNextSceneId = eNextSceneId;

	if (FAILED(ReadyLayerBackGround()))
		return E_FAIL;

	/* 다음 레벨에대한 자원준비를 할 수 있도록 한다. */
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
	m_pLoader->PrintText();

	return S_OK;
}

HRESULT LoadingScene::ReadyLayerBackGround()
{
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
