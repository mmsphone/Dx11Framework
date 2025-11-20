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
#include "SMG_Projectile.h"
#include "Weapon_AR.h"
#include "Worm_Projectile.h"
#include "BloodHitEffect.h"
#include "Spawner.h"
#include "TriggerBox.h"
#include "Door.h"
#include "Panel.h"
#include "Console.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"

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
	lstrcpy(m_szLoading, TEXT("폰트 로딩중..."));
	m_pEngineUtility->AddFont(L"Font_Default", L"../bin/Resources/Fonts/155ex.spritefont");

	lstrcpy(m_szLoading, TEXT("텍스처 로딩중..."));

	lstrcpy(m_szLoading, TEXT("셰이더 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxPosTex"), Shader::Create(TEXT("../bin/Shader/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), Shader::Create(TEXT("../bin/Shader/Shader_VtxAnimMesh.hlsl"), VTXSKINMESH::Elements, VTXSKINMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_InstancingPos"), Shader::Create(TEXT("../bin/Shader/Shader_InstancingPos.hlsl"), VTXPOS_INSTANCEPARTICLE::Elements, VTXPOS_INSTANCEPARTICLE::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxProjectile"), Shader::Create(TEXT("../bin/Shader/Shader_VtxProjectile.hlsl"), VTXPOS_INSTANCEPARTICLE::Elements, VTXPOS_INSTANCEPARTICLE::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Shader_VtxBloodHit"), Shader::Create(TEXT("../bin/Shader/Shader_VtxBloodHit.hlsl"), VTXPOSTEX_INSTANCEWORLD::Elements, VTXPOSTEX_INSTANCEWORLD::iNumElements))))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("모델 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("VIBufferRect"), VIBufferRect::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("컴포넌트 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("StateMachine"), StateMachine::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("AIController"), AIController::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Info"), Info::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("Physics"), Physics::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("CollisionAABB"), Collision::Create(COLLISIONTYPE_AABB))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("CollisionOBB"), Collision::Create(COLLISIONTYPE_OBB))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("CollisionSphere"), Collision::Create(COLLISIONTYPE_SPHERE))))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("객체원형 로딩중..."));
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("FreeCam"), FreeCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("FixedCam"), FixedCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("ChaseCam"), ChaseCam::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("UIImage"), UIImage::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("UILabel"), UILabel::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::STATIC, TEXT("UIButton"), UIButton::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("맵 데이터 로딩 중..."));
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
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodCore"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodCore.png"), 1))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodSprite"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodSprite.png"), 1))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodSpray1"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodTexture1.png"), 1))))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodSpray2"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodTexture2.png"), 1))))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("모델 로딩중..."));
	ModelData* model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/GameScene1/GameScene1.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_GameScene1_Map"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Player_Male/Player_Male.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Player"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Drone/Monster_Drone.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Drone"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Worm/Monster_Worm.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Worm"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Shieldbug/Monster_Shieldbug.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Shieldbug"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Weapon_AR/Weapon_AR.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Weapon_AR"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Door/Door.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Door"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Panel/Panel.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Panel"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Console/Console.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Console"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;

	{
		VIBufferInstancingPoint::POINT_INSTANCE_DESC pointDesc{};
		pointDesc.iNumInstance = 1;
		pointDesc.vScale = _float2(0.3f, 0.3f);
		pointDesc.vCenter = _float3(0.f, 0.f, 0.f);
		pointDesc.vPivot = _float3(0.f, 0.f, 0.f);
		pointDesc.vRange = _float3(0.0f, 0.0f, 0.0f);
		pointDesc.vLifeTime = _float2(2.f, 2.f);
		pointDesc.vSpeed = _float2(2.f, 4.f);
		pointDesc.isLoop = true;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_Particle_Bullet"), VIBufferInstancingPoint::Create(&pointDesc))))
			return E_FAIL;
	}
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;
		rectDesc.vScale = _float2(1.f, 1.f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);
		rectDesc.vLifeTime = _float2(0.25f, 0.25f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodCore"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;
		rectDesc.vScale = _float2(1.f, 1.f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);
		rectDesc.vLifeTime = _float2(0.25f, 0.25f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSprite"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 400;
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.1f, 0.1f, 0.1f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(2.0f, 5.0f);
		rectDesc.vLifeTime = _float2(0.15f, 0.15f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSpray1"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 400;
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.1f, 0.1f, 0.1f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(2.0f, 5.0f);
		rectDesc.vLifeTime = _float2(0.15f, 0.15f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSpray2"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}

	lstrcpy(m_szLoading, TEXT("객체원형 로딩중..."));
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
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("SMG_Projectile"), SMG_Projectile::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Weapon_AR"), Weapon_AR::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Worm_Projectile"), Worm_Projectile::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("BloodHitEffect"), BloodHitEffect::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Door"), Door::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Panel"), Panel::Create())))
		return E_FAIL;
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Console"), Console::Create())))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("맵 데이터 로딩 중..."));
	std::vector<MAP_OBJECTDATA> mapData = m_pEngineUtility->LoadMapData("../bin/data/GameScene1_Map.dat");
	std::unordered_map<std::string, std::pair<std::wstring, std::wstring>> nameMap = {
		{ "GameScene1", {		L"GameScene1_Map",	L"Map" } },
		{ "Player_Male", {		L"Player",			L"Player" } },
		{ "Door", {				L"Door",			L"Door" } },
		{ "Panel", {			L"Panel",			L"Panel" } },
		{ "Console", {			L"Console",			L"Console" } },
	};
	if (FAILED(LoadMapObjects(SCENE::GAMEPLAY, mapData, nameMap)))
		return E_FAIL;

	lstrcpy(m_szLoading, TEXT("지형 로딩 중..."));
	m_pEngineUtility->LoadCells("../bin/data/GameScene1_Navigation.dat");
	
	lstrcpy(m_szLoading, TEXT("몬스터 스포너 로딩 중..."));
	{// 스포너 0
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-110.f, 18.f, -172.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-109.f, 18.f, -170.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-106.f, 18.f, -172.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 1
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-121.f, 18.f, -157.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-120.f, 18.f, -155.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-119.f, 18.f, -157.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 2
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-118.5f, 18.f, -134.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-122.5f, 19.5f, -138.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-126.f, 18.f, -125.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-126.5f, 19.5f, -116.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 3
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-130.f, 17.f, -90.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-127.5f, 17.f, -85.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-134.f, 17.f, -84.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-122.f, 17.f, -87.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-139.3f, 17.f, -85.7f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 4
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-127.f, 17.f, -69.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-130.f, 17.f, -76.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-135.f, 17.f, -69.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-142.f, 17.f, -63.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-145.f, 17.f, -69.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 5
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-138.f, 17.f, -53.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-146.f, 17.f, -50.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-138.f, 17.f, -45.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-141.f, 17.f, -36.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-154.f, 17.f, -45.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-154.f, 17.f, -36.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-150.f, 17.f, -26.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-143.f, 16.f, -29.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 6
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-134.f, 16.f, 3.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-125.f, 16.f, 4.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-132.5f, 16.f, 13.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-126.f, 16.5f, 11.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-128.f, 16.5f, 20.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-122.f, 16.f, 15.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-127.f, 16.5f, 16.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-122.f, 16.5f, 22.5f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	{// 스포너 7
		_float3 spawnRandomRange = { 1.f, 0.f, 1.f };
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-134.5f, 16.f, 2.5f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-125.f, 16.f, 4.f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-132.5f, 16.f, 18.f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-116.f, 16.f, 12.f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-132.f, 16.f, 9.f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-122.5f, 16.5f, 12.f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-117.f, 16.f, 17.f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-127.f, 16.5f, 12.5f, 1.f), spawnRandomRange);
		m_pEngineUtility->AddSpawner(pSpawner);
	}


	lstrcpy(m_szLoading, TEXT("트리거 박스 로딩 중..."));			   
	m_pEngineUtility->LoadTriggerBoxes("../bin/data/GameScene1_TriggerBoxes.dat");
	vector<TriggerBox*> TriggerBoxes = m_pEngineUtility->GetTriggerBoxes();
	TriggerBoxes[0]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(0); });
	TriggerBoxes[1]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(1); });
	TriggerBoxes[2]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(2); });
	TriggerBoxes[3]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(3); });
	TriggerBoxes[4]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(4); });
	TriggerBoxes[5]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(5); });
	TriggerBoxes[6]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(6); });

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

		//트랜스폼 행렬
		if (auto pTransform = dynamic_cast<Transform*>(pObject->FindComponent(TEXT("Transform"))))
		{
			_vector right = XMLoadFloat4((_float4*)&obj.worldMatrix.m[0]);
			_vector up = XMLoadFloat4((_float4*)&obj.worldMatrix.m[1]);
			_vector look = XMLoadFloat4((_float4*)&obj.worldMatrix.m[2]);
			_vector pos = XMLoadFloat4((_float4*)&obj.worldMatrix.m[3]);

			pTransform->SetState(MATRIXROW_RIGHT, right);
			pTransform->SetState(MATRIXROW_UP, up);
			pTransform->SetState(MATRIXROW_LOOK, look);
			pTransform->SetState(MATRIXROW_POSITION, pos);

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
