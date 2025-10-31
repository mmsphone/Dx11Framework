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

	if (FAILED(ReadyLayerCamera(TEXT("Cam"))))
		return E_FAIL;

	if (FAILED(ReadyLayerUI()))
		return E_FAIL;

	std::vector<MAP_OBJECTDATA> mapData = m_pEngineUtility->LoadMapData("../bin/data/LogoScene.dat");
	if (FAILED(LoadMapObjects(mapData)))
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

HRESULT LogoScene::ReadyLayerCamera(const _tchar* pLayerTag)
{
	FixedCam::FIXEDCAM_DESC			Desc{};

	Desc.vEye = _float3(-15.f, 5.f, 50.f);
	Desc.vAt = _float3(-15.f, 0.f, 0.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 2000.f;
	Desc.fSpeedPerSec = 40.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);

	if (FAILED(m_pEngineUtility->AddObject(SCENE::LOGO, TEXT("FixedCam"), SCENE::LOGO, pLayerTag, &Desc)))
		return E_FAIL;

	return S_OK;
}

HRESULT LogoScene::ReadyLayerUI()
{
	m_pEngineUtility->AddObject(SCENE::LOGO, TEXT("UIStart"), SCENE::LOGO, TEXT("UIStart"));

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

HRESULT LogoScene::LoadMapObjects(const std::vector<MAP_OBJECTDATA>& mapData)
{
	//	filename				Prototype				Layer
	std::unordered_map<std::string, std::pair<std::wstring, std::wstring>> nameMap = {
		{ "ItemInCage_Mesh0",	{ L"CageCamera",		L"CageCamera" } }
	};

	for (auto& obj : mapData)
	{
		const std::string& name = obj.objectName;
		const std::string& path = obj.modelPath;

		auto it = nameMap.find(name);
		if (it == nameMap.end())
			continue;


		std::wstring prototype = it->second.first;
		std::wstring layer = it->second.second;

		//프로토타입 체크
		if (m_pEngineUtility->HasPrototype(SCENE::LOGO, prototype) == false)
		{
			std::wstring msg = L"unknown Prototype :" + prototype + L"\n";
			OutputDebugStringW(msg.c_str());
			continue;
		}

		//오브젝트 추가
		m_pEngineUtility->AddObject(SCENE::LOGO, prototype, SCENE::LOGO, layer);
		Layer* pLayer = m_pEngineUtility->FindLayer(SCENE::LOGO, layer);
		if (pLayer == nullptr)
			continue;
		Object* pObject = pLayer->GetAllObjects().back();
		if (pObject == nullptr)
			continue;

		// 모델 데이터 로드
		Model* pModel = dynamic_cast<Model*>(pObject->FindComponent(TEXT("Model")));
		if (pModel)
		{
			ModelData* pModelData = m_pEngineUtility->LoadNoAssimpModel(obj.modelPath.c_str());
			if (pModelData)
			{
				pModel->SetModelData(pModelData);
				pModel->SetBinPath(obj.modelPath);
			}
		}

		// 트랜스폼 적용
		Transform* pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform")));
		if (pTransform)
		{
			_vector right = XMLoadFloat4((_float4*)&obj.worldMatrix.m[0]);
			_vector up = XMLoadFloat4((_float4*)&obj.worldMatrix.m[1]);
			_vector look = XMLoadFloat4((_float4*)&obj.worldMatrix.m[2]);
			_vector pos = XMLoadFloat4((_float4*)&obj.worldMatrix.m[3]);

			pTransform->SetState(RIGHT, right);
			pTransform->SetState(UP, up);
			pTransform->SetState(LOOK, look);
			pTransform->SetState(POSITION, pos);


		}
	}

	return S_OK;
}
