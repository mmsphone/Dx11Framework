#include "GameScene.h"

#include "EngineUtility.h"
#include "FreeCam.h"

GameScene::GameScene()
	: Scene{ }
{
}

HRESULT GameScene::Initialize()
{
	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(ReadyLayerPlayer(TEXT("Layer_Player"))))
		return E_FAIL;

	if (FAILED(ReadyLayerMonster(TEXT("Layer_Monster"))))
		return E_FAIL;

	if (FAILED(ReadyLayerBackGround(TEXT("Layer_BackGround"))))
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

	LightDesc.eType = LIGHT::DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT GameScene::ReadyLayerCamera(const _tchar* pLayerTag)
{
	FreeCam::FREECAM_DESC			Desc{};

	Desc.vEye = _float3(0.f, 10.f, -6.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	Desc.fSensor = 0.1f;
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);

	if (FAILED(m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("Prototype_Object_FreeCam"), SCENE::GAMEPLAY, pLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT GameScene::ReadyLayerPlayer(const _tchar* pLayerTag)
{
	if (FAILED(m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("Prototype_Object_Player"), SCENE::GAMEPLAY, pLayerTag)))
		return E_FAIL;

	return S_OK;
}

HRESULT GameScene::ReadyLayerMonster(const _tchar* pLayerTag)
{
	for (size_t i = 0; i < 10; i++)
	{
		if (FAILED(m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("Prototype_Object_Monster"), SCENE::GAMEPLAY, pLayerTag)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT GameScene::ReadyLayerBackGround(const _tchar* pLayerTag)
{
	if (FAILED(m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("Prototype_Object_Terrain"), SCENE::GAMEPLAY, pLayerTag)))
		return E_FAIL;

	if (FAILED(m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("Prototype_Object_Sky"), SCENE::GAMEPLAY, pLayerTag)))
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
