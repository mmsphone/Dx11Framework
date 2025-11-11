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
	LIGHT_DESC		LightDesc{};
	SHADOW_DESC		ShadowDesc{};

	LightDesc.eType = LIGHT_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.3f, 0.3f, 0.3f, 1.f);
	LightDesc.vAmbient = _float4(0.f, 0.f, 0.f, 1.f);
	LightDesc.vSpecular = _float4(0.f, 0.f, 0.f, 1.f);
	
	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_POINT;
	LightDesc.vPosition = _float4(-95.0f, 25.0f, -170.f, 1.f);
	LightDesc.fRange = 15.0f;
	LightDesc.vDiffuse = _float4(0.4f, 0.4f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.5f, 1.f);
	LightDesc.vSpecular = _float4(0.f, 0.f, 0.1f, 1.f);
	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;
	ShadowDesc.vEye = _float3(-95.f, 25.f, -170.f);
	ShadowDesc.vAt = _float3(-94.f, 0.f, -171.f);
	ShadowDesc.fFovy = XMConvertToRadians(120.0f);
	ShadowDesc.fAspect = static_cast<_float>(g_iWinSizeX) / g_iWinSizeY;
	ShadowDesc.fNear = 0.1f;
	ShadowDesc.fFar = 15.0f;
	if (FAILED(m_pEngineUtility->AddShadowLight(ShadowDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_POINT;
	LightDesc.vPosition = _float4(-110.0f, 25.0f, -170.f, 1.f);
	LightDesc.fRange = 15.0f;
	LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	LightDesc.vAmbient = _float4(0.5f, 0.2f, 0.2f, 1.f);
	LightDesc.vSpecular = _float4(0.1f, 0.f, 0.f, 1.f);
	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;
	ShadowDesc.vEye = _float3(-110.f, 25.f, -170.f);
	ShadowDesc.vAt = _float3(-109.f, 0.f, -171.f);
	ShadowDesc.fFovy = XMConvertToRadians(120.0f);
	ShadowDesc.fAspect = static_cast<_float>(g_iWinSizeX) / g_iWinSizeY;
	ShadowDesc.fNear = 0.1f;
	ShadowDesc.fFar = 15.0f;
	if (FAILED(m_pEngineUtility->AddShadowLight(ShadowDesc)))
		return E_FAIL;
	
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
	XMStoreFloat3(&Desc.vAt, pTransform->GetState(POSITION));
	XMStoreFloat3(&Desc.vEye, pTransform->GetState(POSITION) + Desc.offset);
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