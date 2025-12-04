#include "Loader.h"

#include "EngineUtility.h"

#include "Layer.h"

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
#include "BloodDieEffect.h"
#include "hackingGameUI.h"
#include "hacking2GameUI.h"
#include "QuestUI.h"
#include "minimapUI.h"
#include "Ammobag.h"
#include "endingUI.h"
#include "Itembox.h"
#include "Grenade.h"
#include "ExplosionEffect.h"

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

HRESULT Loader::LoadingForLogo()
{
	UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"Loadingbar_Front"));
	_float fCur = 0.f;
	_float fMax = 7.f;
	pUI->SetLoadingRatio(0.f);

	m_pEngineUtility->LoadUI("../bin/data/logoUI_background.dat", SCENE::LOGO);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/logoUI_plate.dat", SCENE::LOGO);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/logoUI_default.dat", SCENE::LOGO);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/logoUI_on.dat", SCENE::LOGO);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/logoUI_button.dat", SCENE::LOGO);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/logoUI_text.dat", SCENE::LOGO);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadSound("BGM_lobby", L"../bin/Resources/Sounds/BGM_lobby.wav", false, true);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_isFinished = true;
	return S_OK;
}

HRESULT Loader::LoadingForGamePlay()
{
	UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"Loadingbar_Front"));
	_float fCur = 0.f;
	_float fMax = 179.f;
	pUI->SetLoadingRatio(0.f);

	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodCore"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodCore.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodSprite"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodSprite.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodSpray1"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodTexture1.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_BloodSpray2"), Texture::Create(TEXT("../bin/Resources/Textures/BloodHit/bloodTexture2.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionComet"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/comet.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionParticle"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/fire_particle_7.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionFire"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/flamethrowerfire102.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionBlast"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/fluid_blast.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionDebris1"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_debris_burst_001.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionDebris2"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_debris_burst_002.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow1"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_glow_01.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow2"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_glow_04.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow3"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_glow_05.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow4"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_glow_09.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionRing"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/particle_ring_wave_5.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionSmoke"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/smoke1.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Texture_ExplosionCrater"), Texture::Create(TEXT("../bin/Resources/Textures/Explosion/snow_crater_1.png"), 1))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	
	ModelData* model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/GameScene1/GameScene1.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_GameScene1_Map"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Player_Male/Player_Male.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Player"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Drone/Monster_Drone.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Drone"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Worm/Monster_Worm.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Worm"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Monster_Shieldbug/Monster_Shieldbug.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Shieldbug"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Weapon_AR/Weapon_AR.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Weapon_AR"), Model::Create(MODELTYPE::MODELTYPE_ANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Door/Door.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Door"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Panel/Panel.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Panel"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Console/Console.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Console"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/ammobag/ammobag.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Ammobag"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Itembox/Itembox.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Itembox"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	model = m_pEngineUtility->LoadNoAssimpModel("../bin/Resources/Models/Grenade/Grenade.bin");
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Model_Grenade"), Model::Create(MODELTYPE::MODELTYPE_NONANIM, model))))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);

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
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;
		rectDesc.vScale = _float2(1.f, 1.f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);
		rectDesc.vLifeTime = _float2(0.2f, 0.2f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodCore"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;
		rectDesc.vScale = _float2(1.5f, 1.5f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);
		rectDesc.vLifeTime = _float2(0.2f, 0.2f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSprite"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1000;
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.1f, 0.1f, 0.1f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(1.f, 4.0f);
		rectDesc.vLifeTime = _float2(0.2f, 0.2f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSpray1"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1000;
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.1f, 0.1f, 0.1f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(1.f, 4.0f);
		rectDesc.vLifeTime = _float2(0.2f, 0.2f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSpray2"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;
		rectDesc.vScale = _float2(1.5f, 1.5f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);
		rectDesc.vLifeTime = _float2(0.2f, 0.2f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieCore"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;
		rectDesc.vScale = _float2(2.f, 2.f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);
		rectDesc.vLifeTime = _float2(0.2f, 0.2f);
		rectDesc.isLoop = false;
		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieSprite"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1000;
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.1f, 0.1f, 0.1f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(1.f, 4.0f);
		rectDesc.vLifeTime = _float2(1.f, 1.f);
		rectDesc.isLoop = false;
		rectDesc.repeatable = true;
		rectDesc.repeatDuration = _float2(0.02f, 0.2f);

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY,TEXT("VIBuffer_BloodDieSpray1"),VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1000;
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.1f, 0.1f, 0.1f);
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(1.f, 4.0f);
		rectDesc.vLifeTime = _float2(1.f, 1.f);
		rectDesc.isLoop = false;
		rectDesc.repeatable = true;
		rectDesc.repeatDuration = _float2(0.02f, 0.2f);

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieSpray2"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 1;                   // 큰 판 1장
		rectDesc.vScale = _float2(1.f, 1.f);   // 기본 크기 (반경은 셰이더에서 g_RadiusScale로 곱)
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);
		rectDesc.vRange = _float3(0.f, 0.f, 0.f); // 랜덤 위치 없음
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(0.f, 0.f);      // 움직이지 않음
		rectDesc.vLifeTime = _float2(1.f, 1.f);
		rectDesc.isLoop = false;
		rectDesc.repeatable = false;

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_Explosion"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 300;                 // 불 파편 개수
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);   // 기준은 0,0,0 (Transform에서 옮김)
		rectDesc.vRange = _float3(0.4f, 0.4f, 0.4f); // 퍼지는 랜덤 범위
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(3.f, 7.f);        // 랜덤 속도
		rectDesc.vLifeTime = _float2(0.4f, 0.7f);      // 개별 파편 수명
		rectDesc.isLoop = false;
		rectDesc.repeatable = false;                    // 한 번에 “빵” 하고 끝나는 타입

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY,TEXT("VIBuffer_ExplosionParticle"),VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 300;                 // 불 파편 개수
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);   // 기준은 0,0,0 (Transform에서 옮김)
		rectDesc.vRange = _float3(0.4f, 0.4f, 0.4f); // 퍼지는 랜덤 범위
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(3.f, 7.f);        // 랜덤 속도
		rectDesc.vLifeTime = _float2(0.4f, 0.7f);      // 개별 파편 수명
		rectDesc.isLoop = false;
		rectDesc.repeatable = false;                    // 한 번에 “빵” 하고 끝나는 타입

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionFire"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 300;                 // 불 파편 개수
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);   // 기준은 0,0,0 (Transform에서 옮김)
		rectDesc.vRange = _float3(0.4f, 0.4f, 0.4f); // 퍼지는 랜덤 범위
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(3.f, 7.f);        // 랜덤 속도
		rectDesc.vLifeTime = _float2(0.4f, 0.7f);      // 개별 파편 수명
		rectDesc.isLoop = false;
		rectDesc.repeatable = false;                    // 한 번에 “빵” 하고 끝나는 타입

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionSmoke"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{
		VIBufferInstancingRect::RECT_INSTANCE_DESC rectDesc{};
		rectDesc.iNumInstance = 300;                 // 불 파편 개수
		rectDesc.vScale = _float2(0.05f, 0.1f);
		rectDesc.vCenter = _float3(0.f, 0.f, 0.f);   // 기준은 0,0,0 (Transform에서 옮김)
		rectDesc.vRange = _float3(0.4f, 0.4f, 0.4f); // 퍼지는 랜덤 범위
		rectDesc.vPivot = _float3(0.f, 0.f, 0.f);
		rectDesc.vSpeed = _float2(3.f, 7.f);        // 랜덤 속도
		rectDesc.vLifeTime = _float2(0.4f, 0.7f);      // 개별 파편 수명
		rectDesc.isLoop = false;
		rectDesc.repeatable = false;                    // 한 번에 “빵” 하고 끝나는 타입

		if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionDebris"), VIBufferInstancingRect::Create(&rectDesc))))
			return E_FAIL;
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);

	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("GameScene1_Map"), GameScene1_Map::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Player"), Player::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Drone"), Drone::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Worm"), Worm::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Shieldbug"), Shieldbug::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("SMG_Projectile"), SMG_Projectile::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Weapon_AR"), Weapon_AR::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Worm_Projectile"), Worm_Projectile::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("BloodHitEffect"), BloodHitEffect::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Door"), Door::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Panel"), Panel::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Console"), Console::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("BloodDieEffect"), BloodDieEffect::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("hackingGameUI"), hackingGameUI::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("hacking2GameUI"), hacking2GameUI::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("questUI"), QuestUI::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("minimapUI"), minimapUI::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Ammobag"), Ammobag::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("endingUI"), endingUI::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Itembox"), Itembox::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("Grenade"), Grenade::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);
	if (FAILED(m_pEngineUtility->AddPrototype(SCENE::GAMEPLAY, TEXT("ExplosionEffect"), ExplosionEffect::Create())))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);

	std::vector<MAP_OBJECTDATA> mapData = m_pEngineUtility->LoadMapData("../bin/data/GameScene1_Map.dat");
	UpdateLoadingRatio(pUI, &fCur, fMax);

	std::unordered_map<std::string, std::pair<std::wstring, std::wstring>> nameMap = {
		{ "GameScene1", {		L"GameScene1_Map",	L"Map" } },
		{ "Player_Male", {		L"Player",			L"Player" } },
		{ "Door", {				L"Door",			L"Door" } },
		{ "Panel", {			L"Panel",			L"Panel" } },
		{ "Console", {			L"Console",			L"Console" } },
		{ "ammobag", {			L"Ammobag",			L"Ammobag" } },
		{ "Itembox", {			L"Itembox",			L"Itembox" } },
	};
	if (FAILED(LoadMapObjects(SCENE::GAMEPLAY, mapData, nameMap)))
		return E_FAIL;
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadCells("../bin/data/GameScene1_Navigation.dat");
	UpdateLoadingRatio(pUI, &fCur, fMax);

	{// 스포너 0
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-110.f, 18.f, -172.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-109.f, 18.f, -170.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-106.f, 18.f, -172.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 1
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-121.f, 18.f, -157.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-120.f, 18.f, -155.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-119.f, 18.f, -157.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 2
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-118.5f, 18.f, -134.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-126.f, 18.f, -125.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 3
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-130.f, 17.f, -90.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-128.5f, 17.f, -86.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-132.f, 17.f, -85.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-130.f, 17.f, -87.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-131.f, 17.f, -86.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 4
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Worm", L"Worm", XMVectorSet(-127.f, 17.f, -69.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-130.f, 17.f, -76.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-135.f, 17.f, -69.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-142.f, 17.f, -63.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-145.f, 17.f, -69.f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 5
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-138.f, 17.f, -53.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-146.f, 17.f, -50.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-138.f, 17.f, -45.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-141.f, 17.f, -36.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-154.f, 17.f, -45.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-154.f, 17.f, -36.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-150.f, 17.f, -26.5f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 6
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-134.f, 16.f, 3.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-125.f, 16.f, 4.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-132.5f, 16.f, 13.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-126.f, 16.5f, 11.5f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-128.f, 16.5f, 20.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Drone", L"Drone", XMVectorSet(-122.f, 16.f, 15.f, 1.f));
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-127.f, 16.5f, 16.5f, 1.f));
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);
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
	UpdateLoadingRatio(pUI, &fCur, fMax);
	{// 스포너 8
		_float3 spawnRandomRange = { 1.f, 0.f, 1.f };
		Spawner* pSpawner = Spawner::Create();
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-149.f, 16.f, 3.5f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-151.f, 16.f, 5.5f, 1.f), spawnRandomRange);
		pSpawner->AddSpawnerMob(SCENE::GAMEPLAY, L"Shieldbug", L"Shieldbug", XMVectorSet(-153.f, 16.f, 3.5f, 1.f), spawnRandomRange);
		m_pEngineUtility->AddSpawner(pSpawner);
	}
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadTriggerBoxes("../bin/data/GameScene1_TriggerBoxes.dat");
	vector<TriggerBox*> TriggerBoxes = m_pEngineUtility->GetTriggerBoxes();
	TriggerBoxes[0]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(0); });
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[1]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(1); });
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[2]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(2); });
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[3]->SetTriggerFunction([]() { 
		EngineUtility::GetInstance()->Spawn(3); 
		static_cast<QuestUI*>(EngineUtility::GetInstance()->FindUI(L"questUI"))->SetNpcType(QUEST_NPC_MALE);
		EngineUtility::GetInstance()->StartQuest(2);
		EngineUtility::GetInstance()->SetMainQuestId(2);
	});
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[4]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(4); });
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[5]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(5); });
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[6]->SetTriggerFunction([]() { 
		EngineUtility::GetInstance()->Spawn(6); 
		static_cast<QuestUI*>(EngineUtility::GetInstance()->FindUI(L"questUI"))->SetNpcType(QUEST_NPC_MALE);
		EngineUtility::GetInstance()->StartQuest(3);
		EngineUtility::GetInstance()->SetMainQuestId(3);
		});
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[8]->SetTriggerFunction([]() { EngineUtility::GetInstance()->Spawn(8); });
	UpdateLoadingRatio(pUI, &fCur, fMax);

	TriggerBoxes[0]->SetTriggerTag(L"trigger0");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[1]->SetTriggerTag(L"trigger1");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[2]->SetTriggerTag(L"trigger2");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[3]->SetTriggerTag(L"trigger3");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[4]->SetTriggerTag(L"trigger4");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[5]->SetTriggerTag(L"trigger5");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[6]->SetTriggerTag(L"trigger6");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[7]->SetTriggerTag(L"trigger7");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[8]->SetTriggerTag(L"trigger8");
	UpdateLoadingRatio(pUI, &fCur, fMax);
	TriggerBoxes[9]->SetTriggerTag(L"trigger9");
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadUI("../bin/data/minimap.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/playerUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadUI("../bin/data/hackingUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/hacking2UI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/ammobagUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/itemboxUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadUI("../bin/data/waveUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/questUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadUI("../bin/data/hackingGame.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadUI("../bin/data/hacking2Game.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	m_pEngineUtility->LoadUI("../bin/data/endingUI.dat", SCENE::GAMEPLAY);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	//BGM
	m_pEngineUtility->LoadSound("BGM_game", L"../bin/Resources/Sounds/BGM_game.mp3", false, true);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("BGM_wave", L"../bin/Resources/Sounds/BGM_wave.mp3", false, true);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("BGM_minigame2downloading", L"../bin/Resources/Sounds/BGM_minigame2downloading.wav", false, true);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("BGM_waveBeep", L"../bin/Resources/Sounds/BGM_waveBeep.wav", false, true);
	UpdateLoadingRatio(pUI, &fCur, fMax);

	//FBX
	m_pEngineUtility->LoadSound("FBX_doorMove", L"../bin/Resources/Sounds/FBX_doorMove.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneRoar1", L"../bin/Resources/Sounds/FBX_droneRoar1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneRoar2", L"../bin/Resources/Sounds/FBX_droneRoar2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_explosion", L"../bin/Resources/Sounds/FBX_explosion.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_gameclear", L"../bin/Resources/Sounds/FBX_gameclear.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_gameover", L"../bin/Resources/Sounds/FBX_gameover.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_minigameButton", L"../bin/Resources/Sounds/FBX_minigameButton.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_minigame2Button", L"../bin/Resources/Sounds/FBX_minigame2Button.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_minigame2Show", L"../bin/Resources/Sounds/FBX_minigame2Show.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_pickupItembox", L"../bin/Resources/Sounds/FBX_pickupItembox.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_pickupAmmobag", L"../bin/Resources/Sounds/FBX_pickupAmmobag.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_reload", L"../bin/Resources/Sounds/FBX_reload.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_shieldbugRoar1", L"../bin/Resources/Sounds/FBX_shieldbugRoar1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_shootAR1", L"../bin/Resources/Sounds/FBX_shootAR1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_shootAR2", L"../bin/Resources/Sounds/FBX_shootAR2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_throw", L"../bin/Resources/Sounds/FBX_throw.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneDie1", L"../bin/Resources/Sounds/FBX_droneDie1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneDie2", L"../bin/Resources/Sounds/FBX_droneDie2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneDie3", L"../bin/Resources/Sounds/FBX_droneDie3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneDie4", L"../bin/Resources/Sounds/FBX_droneDie4.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit1", L"../bin/Resources/Sounds/FBX_droneHit1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit2", L"../bin/Resources/Sounds/FBX_droneHit2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit3", L"../bin/Resources/Sounds/FBX_droneHit3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit4", L"../bin/Resources/Sounds/FBX_droneHit4.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit5", L"../bin/Resources/Sounds/FBX_droneHit5.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit6", L"../bin/Resources/Sounds/FBX_droneHit6.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit7", L"../bin/Resources/Sounds/FBX_droneHit7.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit8", L"../bin/Resources/Sounds/FBX_droneHit8.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit9", L"../bin/Resources/Sounds/FBX_droneHit9.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit10", L"../bin/Resources/Sounds/FBX_droneHit10.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit11", L"../bin/Resources/Sounds/FBX_droneHit11.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_droneHit12", L"../bin/Resources/Sounds/FBX_droneHit12.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_shieldbugHit1", L"../bin/Resources/Sounds/FBX_shieldbugHit1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_shieldbugHit2", L"../bin/Resources/Sounds/FBX_shieldbugHit2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_wormDie1", L"../bin/Resources/Sounds/FBX_wormDie1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepConcrete1", L"../bin/Resources/Sounds/FBX_playerStepConcrete1.wav", false, false); //
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepConcrete2", L"../bin/Resources/Sounds/FBX_playerStepConcrete2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepConcrete3", L"../bin/Resources/Sounds/FBX_playerStepConcrete3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepConcrete4", L"../bin/Resources/Sounds/FBX_playerStepConcrete4.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepMetal1", L"../bin/Resources/Sounds/FBX_playerStepMetal1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepMetal2", L"../bin/Resources/Sounds/FBX_playerStepMetal2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepMetal3", L"../bin/Resources/Sounds/FBX_playerStepMetal3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerStepMetal4", L"../bin/Resources/Sounds/FBX_playerStepMetal4.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerAFK1", L"../bin/Resources/Sounds/FBX_playerAFK1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerAFK2", L"../bin/Resources/Sounds/FBX_playerAFK2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerAFK3", L"../bin/Resources/Sounds/FBX_playerAFK3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHackComplete1", L"../bin/Resources/Sounds/FBX_playerHackComplete1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHackComplete2", L"../bin/Resources/Sounds/FBX_playerHackComplete2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHackComplete3", L"../bin/Resources/Sounds/FBX_playerHackComplete3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHolyshit", L"../bin/Resources/Sounds/FBX_playerHolyshit.wav", false, false); //
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit1", L"../bin/Resources/Sounds/FBX_playerHit1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit2", L"../bin/Resources/Sounds/FBX_playerHit2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit3", L"../bin/Resources/Sounds/FBX_playerHit3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit4", L"../bin/Resources/Sounds/FBX_playerHit4.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit5", L"../bin/Resources/Sounds/FBX_playerHit5.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit6", L"../bin/Resources/Sounds/FBX_playerHit6.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerHit7", L"../bin/Resources/Sounds/FBX_playerHit7.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerDie1", L"../bin/Resources/Sounds/FBX_playerDie1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerDie2", L"../bin/Resources/Sounds/FBX_playerDie2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerThrow1", L"../bin/Resources/Sounds/FBX_playerThrow1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerThrow2", L"../bin/Resources/Sounds/FBX_playerThrow2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerReload1", L"../bin/Resources/Sounds/FBX_playerReload1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerReload2", L"../bin/Resources/Sounds/FBX_playerReload2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerOpenDoor", L"../bin/Resources/Sounds/FBX_playerOpenDoor.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerYes", L"../bin/Resources/Sounds/FBX_playerYes.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_raider", L"../bin/Resources/Sounds/FBX_raider.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerShell1", L"../bin/Resources/Sounds/FBX_playerShell1.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerShell2", L"../bin/Resources/Sounds/FBX_playerShell2.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_playerShell3", L"../bin/Resources/Sounds/FBX_playerShell3.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_questGoNext", L"../bin/Resources/Sounds/FBX_questGoNext.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_quest1End", L"../bin/Resources/Sounds/FBX_quest1End.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_quest3End", L"../bin/Resources/Sounds/FBX_quest3End.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_questDownload", L"../bin/Resources/Sounds/FBX_questDownload.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_questEscape", L"../bin/Resources/Sounds/FBX_questEscape.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_questGoDeeper", L"../bin/Resources/Sounds/FBX_questGoDeeper.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_questHacking", L"../bin/Resources/Sounds/FBX_questHacking.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_questPickup", L"../bin/Resources/Sounds/FBX_questPickup.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);
	m_pEngineUtility->LoadSound("FBX_swing", L"../bin/Resources/Sounds/FBX_swing.wav", false, false);
	UpdateLoadingRatio(pUI, &fCur, fMax);

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

			if (auto* pPlayer = dynamic_cast<Player*>(pObject))
			{
				_int cellIndex = -1;
				if (m_pEngineUtility->IsInCell(pTransform->GetState(MATRIXROW_POSITION), &cellIndex))
				{
					_vector fixedPos = XMVectorZero();
					m_pEngineUtility->SetHeightOnCell(pos, &fixedPos);
					pTransform->SetState(MATRIXROW_POSITION, fixedPos);
				}
			}
		}		
	}

	return S_OK;
}

void Loader::UpdateLoadingRatio(UIImage* ui, _float* fCur, const _float fMax)
{
	*fCur += 1.f;
	ui->SetLoadingRatio(*fCur / fMax);
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

	SafeRelease(m_pEngineUtility);
}
