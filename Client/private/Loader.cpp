#include "Loader.h"

#include "EngineUtility.h"

#include "Layer.h"

#include "FreeCam.h"
#include "FixedCam.h"
#include "ChaseCam.h"

#include "GameScene1_Map.h"
#include "Player.h"
#include "Drone.h"
#include "Worm.h"
#include "Shieldbug.h"

Loader::Loader()
	: m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

_uint APIENTRY ThreadMain(void* pArg)
{
	Loader* pLoader = static_cast<Loader*>(pArg);

	if (FAILED(pLoader->Loading()))
		return 1;

	return 0;
}

HRESULT Loader::Initialize(SCENE eNextSceneId)
{
	m_eNextLevelID = eNextSceneId;

	InitializeCriticalSection(&m_CriticalSection);

	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	return S_OK;
}

HRESULT Loader::Loading()
{
	/* 컴객체를 초기화한다. */
	CoInitializeEx(nullptr, 0);

	EnterCriticalSection(&m_CriticalSection);

	HRESULT		hr = {};

	/* 이제 여기에서 레벨에 맞는 로딩을 수행하면 된다. */

	switch (m_eNextLevelID)
	{
	case SCENE::LOGO:
		hr = LoadingForLogo();
		break;
	case SCENE::GAMEPLAY:
		hr = LoadingForGamePlay();
		break;
	}

	LeaveCriticalSection(&m_CriticalSection);
	CoUninitialize();

	return hr;
}

_bool Loader::isFinished() const
{
	return m_isFinished;
}

void Loader::PrintText()
{
	SetWindowText(g_hWnd, m_szLoading);
}

HRESULT Loader::LoadingForLogo()
{
	lstrcpy(m_szLoading, TEXT("텍스처 로딩중..."));

	lstrcpy(m_szLoading, TEXT("셰이더 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("Shader_VtxPosTex"), Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	lstrcpy(m_szLoading, TEXT("모델 로딩중..."));
	
	lstrcpy(m_szLoading, TEXT("객체원형 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::LOGO, TEXT("FixedCam"), FixedCam::Create())))
		return E_FAIL;

	//lstrcpy(m_szLoading, TEXT("맵 데이터 로딩 중..."));
	//std::vector<MAP_OBJECTDATA> mapData = m_pEngineUtility->LoadMapData("../bin/data/LogoScene.dat");
	//
	//std::unordered_map<std::string, std::pair<std::wstring, std::wstring>> nameMap = {
	//	{ "ItemInCage_Mesh0", { L"GameScene1_Map", L"GameScene1_Map" } },
	//};
	//if (FAILED(LoadMapObjects(SCENE::LOGO, mapData, nameMap)))
	//	return E_FAIL;

	lstrcpy(m_szLoading, TEXT("로딩 완료"));
	m_isFinished = true;

	return S_OK;
}

HRESULT Loader::LoadingForGamePlay()
{
 	lstrcpy(m_szLoading, TEXT("텍스처 로딩중..."));

	lstrcpy(m_szLoading, TEXT("셰이더 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shader_SphereSky"), Shader::Create(TEXT("../bin/Shader/Shader_SphereSky.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shader_VtxAnimMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"), VTXSKINMESH::Elements, VTXSKINMESH::iNumElements))))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("모델 로딩중..."));
	ModelData* model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/GameScene1/GameScene1.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_GameScene1_Map"), Model::Create(MODELTYPE::NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Player_Male/Player_Male.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Player"), Model::Create(MODELTYPE::ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Drone/Monster_Drone.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Drone"), Model::Create(MODELTYPE::ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Worm/Monster_Worm.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Worm"), Model::Create(MODELTYPE::ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Shieldbug/Monster_Shieldbug.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Shieldbug"), Model::Create(MODELTYPE::ANIM, model))))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("컴포넌트 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("StateMachine"), StateMachine::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("AIController"), AIController::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("객체원형 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("FreeCam"), FreeCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("FixedCam"), FixedCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("ChaseCam"), ChaseCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("GameScene1_Map"), GameScene1_Map::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Player"), Player::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Drone"), Drone::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Worm"), Worm::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shieldbug"), Shieldbug::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("맵 데이터 로딩 중..."));
	std::vector<MAP_OBJECTDATA> mapData = m_pEngineUtility->LoadMapData("../bin/data/GameScene1_Map.dat");
	
	std::unordered_map<std::string, std::pair<std::wstring, std::wstring>> nameMap = {
		{ "GameScene1", {		L"GameScene1_Map",	L"Map" } },
		{ "Player_Male", {		L"Player",			L"Player" } },
		{ "Monster_Drone", {	L"Drone",			L"Drone" } },
		{ "Monster_Worm", {		L"Worm",			L"Worm" } },
		{ "Monster_Shieldbug", {L"Shieldbug",		L"Shieldbug" } },
	};
	if (FAILED(LoadMapObjects(SCENE::GAMEPLAY, mapData, nameMap)))
		return E_FAIL;

	m_pEngineUtility->LoadCells("../bin/data/GameScene1_Navigation.dat");	

	lstrcpy(m_szLoading, TEXT("로딩 완료"));
	m_isFinished = true;

	return S_OK;
}

HRESULT Loader::LoadMapObjects(SCENE sceneId, const std::vector<MAP_OBJECTDATA>& mapData, const std::unordered_map<std::string, std::pair<std::wstring, std::wstring>>& nameMap)
{
	for (auto& obj : mapData)
	{
		const std::string& name = obj.objectName;
		const std::string& path = obj.modelPath;

		auto it = nameMap.find(name);
		if (it == nameMap.end())
		{
			std::string msg = "[Loader] Unknown Object Name: " + name + "\n";
			OutputDebugStringA(msg.c_str());
			continue;
		}

		const std::wstring& prototype = it->second.first;
		const std::wstring& layer = it->second.second;

		// ✅ 프로토타입 체크
		if (!m_pEngineUtility->HasPrototype(sceneId, prototype))
		{
			std::wstring msg = L"[Loader] Unknown Prototype: " + prototype + L"\n";
			OutputDebugStringW(msg.c_str());
			continue;
		}

		// ✅ 오브젝트 생성
		if (FAILED(m_pEngineUtility->AddObject(sceneId, prototype, sceneId, layer)))
		{
			std::wstring msg = L"[Loader] Failed to AddObject: " + prototype + L"\n";
			OutputDebugStringW(msg.c_str());
			continue;
		}

		Layer* pLayer = m_pEngineUtility->FindLayer(sceneId, layer);
		if (!pLayer)
			continue;

		Object* pObject = pLayer->GetAllObjects().back();
		if (!pObject)
			continue;

		////모델 덮기
		//Model* pModel = dynamic_cast<Model*>(pObject->FindComponent(TEXT("Model")));
		//if (pModel)
		//{
		//	ModelData* pModelData = m_pEngineUtility->LoadNoAssimpModel(obj.modelPath.c_str());
		//	if (pModelData)
		//	{
		//		pModel->SetModelData(pModelData);
		//		pModel->SetBinPath(obj.modelPath);
		//	}
		//}

		//트랜스폼 행렬
		if (auto pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform"))))
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

Loader* Loader::Create(SCENE eNextSceneId)
{
	Loader* pInstance = new Loader();

	if (FAILED(pInstance->Initialize(eNextSceneId)))
	{
		MSG_BOX("Failed to Created : Loader");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Loader::Free()
{
	__super::Free();

	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
	DeleteCriticalSection(&m_CriticalSection);

	m_pEngineUtility->DestroyInstance();
}
