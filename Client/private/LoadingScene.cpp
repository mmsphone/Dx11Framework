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
	m_pEngineUtility->PlaySound2D("BGM_loading", 0.5f);
	m_eNextSceneId = eNextSceneId;
	m_pEngineUtility->ClearUIs();
	if (FAILED(ReadyLayerBackGround()))
		return E_FAIL;

	m_pLoader = Loader::Create(eNextSceneId);
	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void LoadingScene::Update(_float fTimeDelta)
{
	{
		// 로딩 아이콘 찾아오기
		UIImage* pCycle = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"Loading_Cycle"));
		if (pCycle)
		{
			// static으로 각도 누적
			static _float sAngleRad = 0.f;

			// 초당 180도 회전 (원하면 90.f, 360.f 등으로 조절해도 됨)
			const _float rotateSpeed = XMConvertToRadians(180.f);

			// 시계 방향 → 수학 좌표계 기준으로는 음수 회전
			sAngleRad -= rotateSpeed * fTimeDelta;

			// 너무 값이 커지지 않게 래핑
			if (sAngleRad <= -XM_2PI)
				sAngleRad += XM_2PI;

			// 절대 각도로 세팅
			pCycle->SetRotationRad(sAngleRad);
		}
	}

 	if (true == m_pLoader->isFinished() && m_pEngineUtility->IsKeyPressed(DIK_RETURN))
	{
		Scene* pNextScene = { nullptr };
		m_pEngineUtility->StopSound("BGM_loading");

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
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Loading_cycle";
		desc.x = 570.f;
		desc.y = 650.f;
		desc.z = 0.f;
		desc.w = 64.f * 1.f;
		desc.h = 64.f * 1.f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Loading/Loading_cycle.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::LOADING, L"UI", &desc);

		UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindLayer(SCENE::LOADING, L"UI")->GetAllObjects().back());
		m_pEngineUtility->AddUI(L"Loading_Cycle", pUI);
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
