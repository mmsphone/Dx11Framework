#include "GameScene.h"

#include "EngineUtility.h"

#include "FreeCam.h"
#include "FixedCam.h"
#include "ChaseCam.h"

#include "Layer.h"

GameScene::GameScene()
	: Scene{ }
{
}

HRESULT GameScene::Initialize()
{
	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera()))
		return E_FAIL;

	return S_OK;
}

void GameScene::Update(_float fTimeDelta)
{
}

HRESULT GameScene::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이레벨입니다."));

	return S_OK;
}

HRESULT GameScene::ReadyLights()
{
	m_pEngineUtility->ReadyLightsFromFile("../bin/data/GameScene1_Light.dat");

	return S_OK;
}

HRESULT GameScene::ReadyLayerCamera()
{
	//FreeCam::FREECAM_DESC			Desc{};
	//Desc.vEye = _float3(0.f, 0.f, 50.f);
	//Desc.vAt = _float3(0.f, 0.f, 0.f);
	//Desc.fFovy = XMConvertToRadians(60.0f);
	//Desc.fNear = 0.1f;
	//Desc.fFar = 500.f;
	//Desc.fSensor = 0.1f;
	//Desc.fSpeedPerSec = 40.f;
	//Desc.fRotationPerSec = XMConvertToRadians(120.0f);
	//
	//if (FAILED(m_pEngineUtility->AddObject(SCENE::STATIC, TEXT("FreeCam"), SCENE::GAMEPLAY, TEXT("Cam"), &Desc)))
	//	return E_FAIL;

	ChaseCam::CHASECAM_DESC			Desc{};
	Object* pPlayer =m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
	if (pPlayer == nullptr)
		return E_FAIL;
	Transform* pTransform = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
	if (pTransform == nullptr)
		return E_FAIL;
	Desc.pTarget = pPlayer;
	Desc.offset = _vector{ 0.f, 10.f, -8.f, 0.f };
	XMStoreFloat3(&Desc.vAt, pTransform->GetState(MATRIXROW_POSITION));
	XMStoreFloat3(&Desc.vEye, pTransform->GetState(MATRIXROW_POSITION) + Desc.offset);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);
	
	if (FAILED(m_pEngineUtility->AddObject(SCENE::STATIC, TEXT("ChaseCam"), SCENE::GAMEPLAY, TEXT("Cam"), &Desc)))
		return E_FAIL;
	
	return S_OK;
}

GameScene* GameScene::Create()
{
	GameScene* pInstance = new GameScene();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : GameScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void GameScene::Free()
{
	__super::Free();
}