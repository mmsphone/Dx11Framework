#include "GameScene.h"

#include "EngineUtility.h"
#include "FreeCam.h"
#include "Layer.h"

GameScene::GameScene()
	: Scene{ }
{
}

HRESULT GameScene::Initialize()
{
	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera(TEXT("Cam"))))
		return E_FAIL;

	std::vector<MAP_OBJECTDATA> mapData = m_pEngineUtility->LoadMapData("../bin/data/GameScene.dat");
	if (FAILED(LoadMapObjects(mapData)))
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

	LightDesc.eType = LIGHT::LIGHT_DIRECTIONAL;
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
	Desc.fFar = 2000.f;
	Desc.fSensor = 0.1f;
	Desc.fSpeedPerSec = 40.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);

	if (FAILED(m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("FreeCam"), SCENE::GAMEPLAY, pLayerTag, &Desc)))
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

HRESULT GameScene::LoadMapObjects(const std::vector<MAP_OBJECTDATA>& mapData)
{
	//	filename				Prototype				Layer
	std::unordered_map<std::string, std::pair<std::wstring, std::wstring>> nameMap = {
		{ "Ground4_Mesh0",		{ L"CarWheel",			L"Car" } },
		{ "Ground4_Mesh1",		{ L"CarWheel",			L"Car" } },
		{ "Ground4_Mesh4",		{ L"CarBody",			L"Car" } },
		{ "Ground4_Mesh5",		{ L"CarTrunk",			L"Car" } },
		{ "Ground4_Mesh6",		{ L"CarTrunkHandle",	L"Car" } },
		{ "Ground4_Mesh7",		{ L"CarTopStuff",		L"Car" } },
		{ "Ground4_Mesh12",		{ L"HouseDoor",			L"HouseDoor" } },
		{ "Ground4_Mesh13",		{ L"HouseDoorHandle",	L"HouseDoor" } },
		{ "Ground4_Mesh26",		{ L"WoodenFence",		L"WoodenFence" } },
		{ "Ground4_Mesh27",		{ L"WoodenFence",		L"WoodenFence" } },
		{ "Ground4_Mesh30",		{ L"Ground",			L"Ground" } },
		{ "Ground4_Mesh31",		{ L"Plant",				L"Plant" } },
		{ "Ground4_Mesh32",		{ L"TreeHead",			L"TreeHead" } },
		{ "Ground4_Mesh33",		{ L"Sky",				L"Sky" } }
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
		if (m_pEngineUtility->HasPrototype(SCENE::GAMEPLAY, prototype) == false)
		{
			std::wstring msg = L"unknown Prototype :" + prototype + L"\n";
			OutputDebugStringW(msg.c_str());
			continue;
		}

		//오브젝트 추가
		m_pEngineUtility->AddObject(SCENE::GAMEPLAY, prototype, SCENE::GAMEPLAY, layer);
		Layer* pLayer = m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, layer);
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
