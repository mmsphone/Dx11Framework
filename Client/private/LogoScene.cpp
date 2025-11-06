#include "LogoScene.h"

#include "EngineUtility.h"
#include "LoadingScene.h"

#include "FixedCam.h"
#include "Layer.h"

LogoScene::LogoScene()
	: Scene{ }
{
}

HRESULT LogoScene::Initialize()
{
	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera()))
		return E_FAIL;

	if (FAILED(ReadyLayerUI()))
		return E_FAIL;

	return S_OK;
}

void LogoScene::Update(_float fTimeDelta)
{
	if (m_pEngineUtility->IsKeyPressed(DIK_RETURN))
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

HRESULT LogoScene::ReadyLights()
{
	LIGHT_DESC		LightDesc{};

	LightDesc.eType = LIGHT::LIGHT_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pEngineUtility->AddLight(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT LogoScene::ReadyLayerCamera()
{
	FixedCam::FIXEDCAM_DESC			Desc{};

	Desc.vEye = _float3(0.f, 0.f, 50.f);
	Desc.vAt = _float3(0.f, 0.f, 0.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 2000.f;
	Desc.fSpeedPerSec = 40.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);

	if (FAILED(m_pEngineUtility->AddObject(SCENE::LOGO, TEXT("FixedCam"), SCENE::LOGO, TEXT("Cam"), &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT LogoScene::ReadyLayerUI()
{
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
