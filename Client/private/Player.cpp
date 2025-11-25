#include "Player.h"

#include "EngineUtility.h"

#include "Weapon_AR.h"
#include "SMG_Projectile.h"
#include "Layer.h"
#include "UI.h"
#include "UILabel.h"
#include "UIImage.h"

Player::Player()
    : ContainerTemplate{}
{
}

Player::Player(const Player& Prototype)
    : ContainerTemplate{ Prototype }
{
}

HRESULT Player::Initialize(void* pArg)
{
    CONTAINER_DESC* pDesc = new CONTAINER_DESC;

    pDesc->iNumParts = 1;
    pDesc->fSpeedPerSec = 4.f;
    pDesc->fRotationPerSec = XMConvertToRadians(180.0f);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    SafeDelete(pDesc);

    SetUpAnimIndexMap();
    SetUpPriorityIndexMap();
    if (FAILED(SetUpStateMachine()))
        return E_FAIL;
    if (FAILED(SetUpInfo()))
        return E_FAIL;

    if (FAILED(ReadyPlayerHitUI()))
        return E_FAIL;

    Physics* pPhysics = static_cast<Physics*>(FindComponent(TEXT("Physics")));
    pPhysics->SetOnGround(true);
    m_isPlayingMinigame = false;

    return S_OK;
}

void Player::Update(_float fTimeDelta)
{
    InputCheck();

    if (m_kbActive)
    {
        HitBack(fTimeDelta);
    }

    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (pInfo != nullptr)
        pInfo->Update(fTimeDelta);

    if (auto pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine"))))
        pSM->Update(fTimeDelta);

    if (auto pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model"))))
        pModel->PlayAnimation(fTimeDelta);

    __super::Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    _vector vPos = pTransform->GetState(MATRIXROW_POSITION);
    m_pEngineUtility->SetActiveLightsByDistance(vPos, 40.f);
    m_pEngineUtility->SetActiveShadowLightsByDistance(vPos, 20.f);

    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));


    UI* hpFront = m_pEngineUtility->FindUI(L"playerHpFront");
    if (hpFront)
    {
        Info* pInfo = static_cast<Info*>(FindComponent(TEXT("Info")));
        INFO_DESC desc = pInfo->GetInfo();
        _float curHP = *std::get_if<_float>(desc.GetPtr("CurHP"));
        _float maxHP = *std::get_if<_float>(desc.GetPtr("MaxHP"));
        _float hpRatio = curHP / maxHP;

        static_cast<UIImage*>(hpFront)->SetHPRatio(hpRatio);
    }
    UpdateHitUI(fTimeDelta);
}

void Player::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this); 
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_SHADOWLIGHT, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Player::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));

    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix"))) 
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW)))) 
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION)))) 
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
        return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (pModel->GetModelData()->bones.size() > 0)
        {
            if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
                continue;

            pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
            pShader->Begin(0);
            pModel->Render(i);
        }
    }

#ifdef _DEBUG
    Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Render();
#endif

    return S_OK;
}

HRESULT Player::RenderShadow(_uint iIndex)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));

    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW, iIndex)))) 
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION, iIndex))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_ShadowLightFarDistance", m_pEngineUtility->GetActiveShadowLightFarDistancePtr(iIndex), sizeof(_float))))
        return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
        pShader->Begin(iIndex+1);
        pModel->Render(i);
    }

    return S_OK;
}

_uint Player::FindAnimIndex(string animName, _uint fallback) const
{
    auto it = m_animIndexMap.find(animName);
    if (it != m_animIndexMap.end())
        return it->second;
    return fallback;
}

_uint Player::FindPriorityIndex(string toStateName, _uint fallback) const
{
    auto it = m_priorityIndexMap.find(toStateName);
    if (it != m_priorityIndexMap.end())
        return it->second;
    return fallback;
}

void Player::SetHit(_vector dirXZ, float power, float duration)
{
    _vector dir = XMVector3Normalize(XMVectorSetY(dirXZ, 0.f));
    if (XMVector3Equal(dir, XMVectorZero()))
        return;

    m_kbDir = dir;
    m_kbPower = max(0.f, power);
    m_kbDuration = max(0.01f, duration);
    m_kbRemain = m_kbDuration;
    m_kbActive = true;

    m_isMove = false;

    m_hitDirWorld = XMVector3Normalize(-dir);
    m_hitUiRemain = m_hitUIDuration;
}

void Player::SetPlayingMinigame(_bool bPlayingMinigame)
{
    m_isPlayingMinigame = bPlayingMinigame;
}

HRESULT Player::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Player"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("StateMachine"), TEXT("StateMachine"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Info"), TEXT("Info"), nullptr, nullptr)))
        return E_FAIL;

    Physics::PHYSICS_DESC desc{};
    desc.worldSizeOffset = 0.1f;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Physics"), TEXT("Physics"), nullptr, &desc)))
        return E_FAIL;

    CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
    OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
    XMStoreFloat3(&OBBDesc.vExtents,_vector{ 0.25f, 1.f, 0.25f } / scaleOffset);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
        return E_FAIL;


    return S_OK;
}

HRESULT Player::SetUpParts()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform)
        return E_FAIL;
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pModel)
        return E_FAIL;

    Weapon_AR::WEAPON_AR_DESC        WeaponDesc{};
    WeaponDesc.pParentMatrix = pTransform->GetWorldMatrixPtr();
    WeaponDesc.pSocketBoneMatrix = pModel->GetSocketBoneMatrixPtr("ValveBiped.Bip01_R_Hand");

    if (FAILED(AddPart(SCENE::GAMEPLAY, TEXT("Weapon_AR"), 0, &WeaponDesc)))
        return E_FAIL;
    
    return S_OK;
}

void Player::SetUpAnimIndexMap()
{
    m_animIndexMap = {
        { "Idle",           0 },
        { "MeleeAttack",    1 },
        { "Throw",          2 },
        { "Help",           3 },
        { "Run",            4 },
        { "Walk",           5 },
        { "Pickup",         6 },
        { "Reload",         7 },
        { "Attack",         8 },
        { "Draw",           9 },
        { "MeleeWeaponAttack", 10 },
        { "ThrowAway",      11 },
    };
}

void Player::SetUpPriorityIndexMap()
{
    m_priorityIndexMap = {
        { "Idle",       0 },
        { "Walk",       1 },
        { "Run",        2 },
        { "Attack",      3 },
        { "Reload",  4 },
    };
}

HRESULT Player::SetUpStateMachine()
{
    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pSM || !pModel)
        return E_FAIL;

    //상태 정의
    pSM->RegisterState("Idle", {
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pPlayer || !pModel)
                return;
            pModel->SetAnimation(pPlayer->FindAnimIndex("Idle"), true, 0.1f);
        },
        nullptr,
        nullptr
    });

    pSM->RegisterState("Walk", {
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pPlayer || !pModel)
                return;
            pModel->SetAnimation(pPlayer->FindAnimIndex("Walk"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return;

            if (auto pPhysics = dynamic_cast<Physics*>(owner->FindComponent(TEXT("Physics"))))
                pPhysics->Update(fTimeDelta);

            pPlayer->Move(fTimeDelta);
            pPlayer->Rotate(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Run", {
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pPlayer || !pModel)
                return;
            pModel->SetAnimation(pPlayer->FindAnimIndex("Run"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = dynamic_cast<Player*>(owner);

            if (auto pPhysics = dynamic_cast<Physics*>(owner->FindComponent(TEXT("Physics"))))
                pPhysics->Update(fTimeDelta);

            pPlayer->Move(fTimeDelta);
            pPlayer->Rotate(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Attack", {
        [](Object* owner, StateMachine* sm){
            Player* pPlayer = static_cast<Player*>(owner);
            Object* pWeapon = pPlayer->m_Parts.at(0);
            Info* partInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
            INFO_DESC desc = partInfo->GetInfo();
            _int curBullet = *std::get_if<_int>(desc.GetPtr("CurBullet"));
            _int curAmmo = *std::get_if<_int>(desc.GetPtr("CurAmmo"));
            if (curBullet <= 0)
            {
                if (curAmmo > 0)
                    sm->SetState("Reload");
                return;
            }

            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
            pModel->SetAnimation(pPlayer->FindAnimIndex("Attack"), false, 0.05f, true);
            pPlayer->Shoot();
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer)
                return;
            pPlayer->Rotate(fTimeDelta);
            Object* pWeapon = pPlayer->m_Parts.at(0);
            if (pWeapon == nullptr)
                return;
            static_cast<Model*>(pWeapon->FindComponent(TEXT("Model")))->PlayAnimation(fTimeDelta);
        },
        nullptr
    });
    pSM->RegisterState("Reload", {
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pPlayer->FindAnimIndex("Reload"), false, 0.05f, false);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
            pPlayer->Rotate(fTimeDelta);

            pModel->PlayAnimation(fTimeDelta);
        },
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            Object* pWeapon = pPlayer->m_Parts.at(0);
            Info* partInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));

            INFO_DESC desc = partInfo->GetInfo();
            _int curBullet = *std::get_if<_int>(desc.GetPtr("CurBullet"));
            _int maxBullet = *std::get_if<_int>(desc.GetPtr("MaxBullet"));
            _int curAmmo = *std::get_if<_int>(desc.GetPtr("CurAmmo"));

            if (curBullet == 0 && curAmmo > 0)
            {
                _int newAmmoCount = curAmmo - 1;
                desc.SetData("CurAmmo", newAmmoCount);

                curBullet = maxBullet;
                desc.SetData("CurBullet", curBullet);
                partInfo->BindInfoDesc(desc);

                auto* util = pPlayer->m_pEngineUtility;
                if (util)
                {
                    util->FindUI(L"ammo_front1")->SetVisible(newAmmoCount >= 1);
                    util->FindUI(L"ammo_front2")->SetVisible(newAmmoCount >= 2);
                    util->FindUI(L"ammo_front3")->SetVisible(newAmmoCount >= 3);
                    util->FindUI(L"ammo_front4")->SetVisible(newAmmoCount >= 4);
                    util->FindUI(L"ammo_front5")->SetVisible(newAmmoCount >= 5);

                    if (auto* bulletLabel = dynamic_cast<UILabel*>(util->FindUI(L"bulletCount")))
                        bulletLabel->SetText(to_wstring(curBullet));

                    if (auto* pMouseBulletCount = dynamic_cast<UIImage*>(util->FindUI(L"MouseBulletCount")))
                    {
                        _float bulletRatio = static_cast<_float>(curBullet) / static_cast<_float>(maxBullet);
                        pMouseBulletCount->SetBulletRatio(bulletRatio);
                    }
                }
            }
        }
        });

    // 전이 정의
    //Idle ->
    pSM->AddTransition("Idle", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*){
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return pPlayer->m_isMove;
        });

    pSM->AddTransition("Idle", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return pPlayer->m_isMove && pPlayer->m_isSprint;
        });

    pSM->AddTransition("Idle", "Attack", FindPriorityIndex("Attack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return pPlayer->m_isShoot;
        });

    //Walk ->
    pSM->AddTransition("Walk", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return !pPlayer->m_isMove;
        });

    pSM->AddTransition("Walk", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*){ 
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return pPlayer->m_isMove && pPlayer->m_isSprint; 
        });

    pSM->AddTransition("Walk", "Attack", FindPriorityIndex("Attack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return pPlayer->m_isShoot;
        });

    //Run ->
    pSM->AddTransition("Run", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return !pPlayer->m_isMove;
        });

    pSM->AddTransition("Run", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return pPlayer->m_isMove && !pPlayer->m_isSprint; 
        });
    
    pSM->AddTransition("Run", "Attack", FindPriorityIndex("Attack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
        Player* pPlayer = dynamic_cast<Player*>(owner);
        if (!pPlayer)
            return false;
        return pPlayer->m_isShoot;
        });

    //Attack ->
    pSM->AddTransition("Attack", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pPlayer->m_isShoot && !pPlayer->m_isMove;
        });

    pSM->AddTransition("Attack", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pPlayer->m_isShoot && pPlayer->m_isMove;
        });

    pSM->AddTransition("Attack", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*) {
        Player* pPlayer = dynamic_cast<Player*>(owner);
        if (!pPlayer)
            return false;
        Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
        if (!pModel)
            return false;
        return pModel->isAnimFinished() && !pPlayer->m_isShoot && pPlayer->m_isMove && pPlayer->m_isSprint;
        });

    pSM->AddTransition("Attack", "Attack", FindPriorityIndex("Attack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;

            return pModel->isAnimFinished() && pPlayer->m_isShoot;
        }, true);

    //Reload ->
    pSM->AddTransition("Reload", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            return pModel->isAnimFinished() && !pPlayer->m_isMove && !pPlayer->m_isShoot;
        });
    pSM->AddTransition("Reload", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            return pModel->isAnimFinished() && pPlayer->m_isMove && !pPlayer->m_isSprint && !pPlayer->m_isShoot;
        });
    pSM->AddTransition("Reload", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            return pModel->isAnimFinished() && pPlayer->m_isMove && pPlayer->m_isSprint && !pPlayer->m_isShoot;
        });
    pSM->AddTransition("Reload", "Attack", FindPriorityIndex("Attack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            return pModel->isAnimFinished() && pPlayer->m_isShoot;
        });


    //초기 상태 설정
    pSM->SetState("Idle");

    return S_OK;
}   

HRESULT Player::SetUpInfo()
{
    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (!pInfo)
        return E_FAIL;

    INFO_DESC desc;
    desc.SetData("MaxHP", _float{ 120.f });
    desc.SetData("CurHP", _float{ 120.f });
    desc.SetData("InvincibleTime", _float{ 0.3f });
    desc.SetData("InvincibleLeft", _float{ 0.f });
    desc.SetData("Time", _float{ 0.f });
    desc.SetData("LastHit", _float{ -999.f });
    desc.SetData("IsHit", _bool{ false });
    desc.SetData("Faction", FACTION_PLAYER);
    pInfo->BindInfoDesc(desc);

    return S_OK;
}

HRESULT Player::ReadyPlayerHitUI()
{
    {
        UI_DESC desc{};
        desc.fSpeedPerSec = 0.f;
        desc.fRotationPerSec = 0.f;
        desc.type = UITYPE::UI_IMAGE;
        desc.name = "PlayerHitDir";
        desc.x = g_iWinSizeX >> 1;
        desc.y = g_iWinSizeY >> 1;
        desc.z = 0.f;
        desc.w = 512.f * 0.3f;
        desc.h = 256.f * 0.3f;
        desc.visible = true;
        desc.imagePath = L"../bin/Resources/Textures/Hit/hitDir.png";

        m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::GAMEPLAY, L"HitUI", &desc);

        UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"HitUI", 0));
        m_pEngineUtility->AddUI(L"HitDir", pUI);
        pUI->SetVisible(false);
    }
    {
        UI_DESC desc{};
        desc.fSpeedPerSec = 0.f;
        desc.fRotationPerSec = 0.f;
        desc.type = UITYPE::UI_IMAGE;
        desc.name = "PlayerHitScreen";
        desc.x = g_iWinSizeX >> 1;
        desc.y = g_iWinSizeY >> 1;
        desc.z = 0.01f;
        desc.w = 1280.f;
        desc.h = 720.f;
        desc.visible = true;
        desc.imagePath = L"../bin/Resources/Textures/Hit/hitScreen.png";

        m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::GAMEPLAY, L"HitUI", &desc);

        UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"HitUI", 1));
        m_pEngineUtility->AddUI(L"HitScreen", pUI);
        pUI->SetVisible(false);
    }
    return S_OK;
}

void Player::InputCheck()
{
    m_isMove = false;

    if (m_isPlayingMinigame)
        return;

    _vector dir = XMVectorZero();

    if (m_pEngineUtility->IsKeyDown(DIK_W)) dir += XMVectorSet(0.f, 0.f, 1.f, 0.f);
    if (m_pEngineUtility->IsKeyDown(DIK_S)) dir += XMVectorSet(0.f, 0.f, -1.f, 0.f);
    if (m_pEngineUtility->IsKeyDown(DIK_A)) dir += XMVectorSet(-1.f, 0.f, 0.f, 0.f);
    if (m_pEngineUtility->IsKeyDown(DIK_D)) dir += XMVectorSet(1.f, 0.f, 0.f, 0.f);

    if (XMVector3Equal(dir, XMVectorZero()))
    {
        m_isMove = false;
        m_aimDir = dir;
    }
    else
    {
        m_isMove = true;
        m_aimDir = XMVector3Normalize(dir);
    }

    m_isSprint = m_pEngineUtility->IsKeyDown(DIK_LSHIFT) ? true : false;
    m_isShoot = m_pEngineUtility->IsMouseDown(MOUSEKEY_LEFTBUTTON) ? true : false;

}

void Player::Move(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Physics* pPhysics = static_cast<Physics*>(FindComponent(TEXT("Physics")));
    if (m_isMove == false)
        return;

    //이동
    _vector prePos = pTransform->GetState(MATRIXROW_POSITION);

    _float moveAmp = 1.f;
    if (m_isSprint) 
        moveAmp = 1.4f;
    pTransform->Translate(m_aimDir, fTimeDelta * moveAmp);
    _vector delta = XMVectorScale(m_aimDir, fTimeDelta * moveAmp);

    _vector movePos = pTransform->GetState(MATRIXROW_POSITION);

    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
    list<Object*> doors = m_pEngineUtility->FindLayer(m_pEngineUtility->GetCurrentSceneId(), TEXT("Door"))->GetAllObjects();
    for (auto& door : doors)
    {
        if (static_cast<Collision*>(door->FindComponent(TEXT("Collision")))->Intersect(pCollision))
        {
            pTransform->SetState(MATRIXROW_POSITION, prePos);
            return;
        }
    }

    _int cellIndex = -1;
    if (m_pEngineUtility->IsInCell(movePos, &cellIndex) == false) // 셀 밖 이동 금지
    {
        if (m_pEngineUtility->IsInCell(prePos, &cellIndex) == false) return;

        _vector slideDelta{};
        if (m_pEngineUtility->GetSlideVectorOnCell(prePos, delta, cellIndex, &slideDelta))
        {
            // 슬라이드 적용
            pTransform->SetState(MATRIXROW_POSITION, prePos + slideDelta);
        }
        else
        {
            // 실패 시 원위치
            pTransform->SetState(MATRIXROW_POSITION, prePos);
        }
    }
    else
    {
        // ----- 히스테리시스 & 계단 허용 파라미터 -----
        const _float snapBand = 0.05f; // 스냅 접촉 밴드(지면으로 붙일 때)
        const _float releaseBand = 0.10f; // 해제 밴드(지면에서 떨어졌다고 판정)
        const _float stepUpMax = 0.20f; // 한 프레임 올라탈 수 있는 최대 높이
        const _float stepDownSnap = 0.20f; // 한 프레임 내려가며 따라붙는 최대 높이

        _float cellPosY = m_pEngineUtility->GetHeightPosOnCell(&movePos, cellIndex);
        _float curY = XMVectorGetY(movePos);
        _float dy = cellPosY - curY; // (+) 위에 있음, (-) 아래에 있음

        // 1) 올라탈 수 없는 큰 단차면 이동 거부(원위치)
        if (dy > stepUpMax)
        {
            pTransform->SetState(MATRIXROW_POSITION, prePos);
            return;
        }

        // 2) 지상 상태일 때
        if (pPhysics->IsOnGround())
        {
            if (dy >= -snapBand) {
                // 거의 같은 높이거나 약간 위: 스냅
                _vector fixedPos{}; m_pEngineUtility->SetHeightOnCell(movePos, &fixedPos);
                pTransform->SetState(MATRIXROW_POSITION, fixedPos);
                pPhysics->SetOnGround(true);
            }
            else if (-dy <= stepDownSnap) {
                // 작게 내려가는 계단: 스냅해서 따라감
                _vector fixedPos{}; m_pEngineUtility->SetHeightOnCell(movePos, &fixedPos);
                pTransform->SetState(MATRIXROW_POSITION, fixedPos);
                pPhysics->SetOnGround(true);
            }
            else if (-dy > releaseBand) {
                pTransform->SetState(MATRIXROW_POSITION, movePos + m_aimDir * 0.1f);
                pPhysics->SetOnGround(false);
            }
            else {
                // 스냅/해제 중간 구간: 마지막 상태 유지(지상)로 안정화
                _vector fixedPos{}; m_pEngineUtility->SetHeightOnCell(movePos, &fixedPos);
                pTransform->SetState(MATRIXROW_POSITION, fixedPos);
                pPhysics->SetOnGround(true);
            }
        }
        // 3) 공중 상태일 때: 더 엄격하게 붙여줌(바닥에 거의 닿았을 때만)
        else
        {
            if (dy >= -snapBand) {
                _vector fixedPos{}; m_pEngineUtility->SetHeightOnCell(movePos, &fixedPos);
                pTransform->SetState(MATRIXROW_POSITION, fixedPos);
                pPhysics->SetOnGround(true);
            }
            else
            {
                // 계속 낙하: XZ 이동만 반영, Y는 물리에서 처리
                pTransform->SetState(MATRIXROW_POSITION, movePos);
                pPhysics->SetOnGround(false);
            }
        }
    }
}

void Player::Rotate(_float fTimeDelta)
{
    Transform* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!tf) { m_shootAimActive = false; return; }

    // 1) 슈팅 에임 보간(최우선, 절대 각도로 고정)
    static const _float shootAimDuration = 0.1f;
    if (m_shootAimActive)
    {
        m_shootAimElapsed += max(0.f, fTimeDelta);
        const float t = clamp(m_shootAimElapsed / shootAimDuration, 0.f, 1.f);

        float delta = m_shootYawTarget - m_shootYawStart;
        while (delta > XM_PI)  delta -= XM_2PI;
        while (delta < -XM_PI) delta += XM_2PI;
        const float yawNow = m_shootYawStart + delta * t;

        tf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, yawNow); // 절대 yaw 세팅

        if (t >= 1.f) {
            tf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, m_shootYawTarget); // 스냅 보정
            m_shootAimActive = false;
        }
        return; // 에임 보간 중엔 이동 회전 차단
    }

    // 2) 이동 중이면 이동 방향으로 증분 회전
    static const _float moveDirDuration = 0.2f;
    if (!m_isShoot && m_isMove)
    {
        // 현재/목표 방향(Y=0 평탄화)
        _vector look = XMVector3Normalize(XMVectorSetY(tf->GetState(MATRIXROW_LOOK), 0.f));
        _vector dir = XMVector3Normalize(XMVectorSetY(m_aimDir, 0.f));
        if (XMVector3Equal(dir, XMVectorZero())) return;

        const float baseOffset = XM_PI; // 모델 정렬(-Z 보정)
        const float curYaw = atan2f(XMVectorGetX(look), XMVectorGetZ(look));
        const float dstYaw = baseOffset + atan2f(XMVectorGetX(dir), XMVectorGetZ(dir));

        float delta = dstYaw - curYaw;
        while (delta > XM_PI)  delta -= XM_2PI;
        while (delta < -XM_PI) delta += XM_2PI;

        // 0.1초 목표로 보간된 증분
        const float t = std::clamp(fTimeDelta / moveDirDuration, 0.f, 1.f);
        const float step = delta * t;

        tf->Turn(_vector{ 0.f,1.f,0.f,0.f }, step);
    }

}

void Player::Shoot()
{
    Info* partInfo = dynamic_cast<Info*>(m_Parts.at(0)->FindComponent(TEXT("Info")));
    if (partInfo)
    {
        INFO_DESC desc = partInfo->GetInfo();
        _int curBullet = *std::get_if<_int>(desc.GetPtr("CurBullet"));
        _int maxBullet = *std::get_if<_int>(desc.GetPtr("MaxBullet"));

        // ★ 재장전은 Reload 상태에서만 처리. 여기서는 0발이면 그냥 발사 안 함.
        if (curBullet <= 0)
            return;

        curBullet -= 1;
        desc.SetData("CurBullet", curBullet);
        partInfo->BindInfoDesc(desc);

        _float bulletRatio = static_cast<_float>(curBullet) / static_cast<_float>(maxBullet);

        if (auto* bulletLabel = dynamic_cast<UILabel*>(m_pEngineUtility->FindUI(L"bulletCount")))
            bulletLabel->SetText(to_wstring(curBullet));

        if (auto* pMouseBulletCount = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"MouseBulletCount")))
            pMouseBulletCount->SetBulletRatio(bulletRatio);
    }

    _float2 mouse = m_pEngineUtility->GetMousePos();
    float d01 = 1.f;

    if (m_pEngineUtility->ReadDepthAtPixel((int)mouse.x, (int)mouse.y, &d01))
    {
        // RS Viewport 가져오기
        D3D11_VIEWPORT vp{};
        UINT vpCount = 1;
        m_pEngineUtility->GetContext()->RSGetViewports(&vpCount, &vp);

        // 픽셀센터
        float sx = std::clamp(mouse.x, 0.0f, vp.Width - 1.0f) + 0.5f;
        float sy = std::clamp(mouse.y, 0.0f, vp.Height - 1.0f) + 0.5f;

        // (현재 사용 중인 행렬 사용 — 필요시 LastVP로 교체 가능)
        _matrix V = m_pEngineUtility->GetTransformMatrix(D3DTS_VIEW);
        _matrix P = m_pEngineUtility->GetTransformMatrix(D3DTS_PROJECTION);

        _vector screen = XMVectorSet(sx, sy, d01, 1.f);
        _vector pickPos = XMVector3Unproject(
            screen,
            vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
            0.0f, 1.0f,
            P, V, XMMatrixIdentity()
        );

        Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
        if (pTransform == nullptr)
            return;
        _vector playerPos = pTransform->GetState(MATRIXROW_POSITION);

        const _float4x4* pPartPosMatPtr = m_Parts.at(0)->GetCombinedWorldMatrix();        
        _matrix pPartPosMat = XMLoadFloat4x4(pPartPosMatPtr);
        const _float4x4* pMuzzlePosMatPtr = static_cast<Model*>(m_Parts.at(0)->FindComponent(TEXT("Model")))->GetSocketBoneMatrixPtr("muzzle");
        _matrix pMuzzlePosMat = XMLoadFloat4x4(pMuzzlePosMatPtr);
        
        _vector muzzlePos = pMuzzlePosMat.r[3];
        muzzlePos = XMVector3TransformCoord(muzzlePos, pPartPosMat);
        _vector moveDir = pickPos - muzzlePos;

        Projectile::PROJECTILE_DESC Desc{};
        Desc.moveDir = moveDir;
        Desc.accTime = 0.f;

        Info* pWeaponInfo = static_cast<Info*>(m_Parts.at(0)->FindComponent(TEXT("Info")));
        _float damage = *std::get_if<_float>(pWeaponInfo->GetInfo().GetPtr("Damage"));
        Desc.damageAmount = damage;
        Desc.faction = FACTION_PLAYER;
        Desc.hitRadius = 1.f;
        Desc.lifeTime = 2.f;
        Desc.fSpeedPerSec = 20.f;

        _uint iCurSceneId = m_pEngineUtility->GetCurrentSceneId();
        if (FAILED(m_pEngineUtility->AddObject(iCurSceneId, TEXT("SMG_Projectile"), iCurSceneId, TEXT("Projectile"), &Desc)))
            return;

        static_cast<Transform*>(m_pEngineUtility->FindLayer(iCurSceneId, TEXT("Projectile"))->GetAllObjects().back()->FindComponent(TEXT("Transform")))->SetState(MATRIXROW_POSITION, muzzlePos);
        {
            Transform* tf = static_cast<Transform*>(FindComponent(TEXT("Transform")));
            if (tf)
            {
                // 목표 방향: 수평화 후 정규화
                _vector dir = XMVector3Normalize(XMVectorSetY(moveDir, 0.f));
                if (!XMVector3Equal(dir, XMVectorZero()))
                {
                    // 현재/목표 yaw (+PI 오프셋: 모델이 -Z를 바라보는 정렬 보정 그대로 유지)
                    _vector look = XMVector3Normalize(XMVectorSetY(tf->GetState(MATRIXROW_LOOK), 0.f));
                    const float baseOffset = XM_PI;

                    const float curYaw = atan2f(XMVectorGetX(look), XMVectorGetZ(look));
                    const float dstYaw = baseOffset + atan2f(XMVectorGetX(dir), XMVectorGetZ(dir));

                    // 최단각으로 정규화
                    float start = curYaw;
                    float target = dstYaw;
                    float delta = target - start;
                    while (delta > XM_PI)  delta -= XM_2PI;
                    while (delta < -XM_PI) delta += XM_2PI;

                    target = start + delta;

                    // 거의 정렬이면 즉시 고정
                    if (fabsf(delta) < 1e-4f) {
                        tf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, target);
                        m_shootAimActive = false;
                    }
                    else {
                        m_shootYawStart = start;
                        m_shootYawTarget = target;
                        m_shootAimElapsed = 0.f;
                        m_shootAimActive = true;
                    }
                }
            }
        }
    }
}

void Player::HitBack(_float fTimeDelta)
{
    m_isMove = false;

    auto* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform)
    {
        // 선형 감쇠(ease-out): 남은 시간 비율
        const float dt = max(0.f, fTimeDelta);
        const float t = (m_kbDuration > 1e-6f) ? (m_kbRemain / m_kbDuration) : 0.f; // 1->0
        const float speed = m_kbPower * t; // 남을수록 감속

        // 충돌/네비 체크
        _vector prePos = pTransform->GetState(MATRIXROW_POSITION);
        pTransform->Translate(m_kbDir, speed * dt);

        _vector newPos = pTransform->GetState(MATRIXROW_POSITION);
        if (!m_pEngineUtility->IsInCell(newPos))
            pTransform->SetState(MATRIXROW_POSITION, prePos);

        Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
        pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
        list<Object*> doors = m_pEngineUtility->FindLayer(m_pEngineUtility->GetCurrentSceneId(), TEXT("Door"))->GetAllObjects();
        for (auto& door : doors)
        {
            if (static_cast<Collision*>(door->FindComponent(TEXT("Collision")))->Intersect(pCollision))
            {
                pTransform->SetState(MATRIXROW_POSITION, prePos);
            }
        }

        m_kbRemain -= dt;
        if (m_kbRemain <= 0.f)
            m_kbActive = false;
    }
    else
    {
        m_kbActive = false;
    }
}

void Player::UpdateHitUI(_float fTimeDelta)
{
    UIImage* hitDir = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"HitDir"));
    UIImage* hitScreen = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"HitScreen"));
    if (hitDir == nullptr || hitScreen == nullptr)
        return;

    if (m_hitUiRemain <= 0.f || XMVector3Equal(m_hitDirWorld, XMVectorZero()))
    {
        hitDir->SetVisible(false);
        hitScreen->SetVisible(false);
        return;
    }
    m_hitUiRemain = max(0.f, m_hitUiRemain - fTimeDelta);
    if (m_hitUiRemain <= 0.f)
    {
        hitDir->SetVisible(false);
        hitScreen->SetVisible(false);
        return;
    }

    hitDir->SetVisible(true);
    hitScreen->SetVisible(true);

    const _float t = m_hitUiRemain / m_hitUIDuration;

    const _float alphaDir = t;
    const _float alphaScreen = t * 0.6f;

    hitDir->SetAlpha(alphaDir);
    hitScreen->SetAlpha(alphaScreen);

    auto* tf = dynamic_cast<Transform*>(hitDir->FindComponent(TEXT("Transform")));
    if (!tf)
        return;

    static bool    s_initBasePos = false;
    static _vector s_basePos = XMVectorZero();
    if (!s_initBasePos)
    {
        s_basePos = tf->GetState(MATRIXROW_POSITION);
        s_initBasePos = true;
    }

    _matrix view = m_pEngineUtility->GetTransformMatrix(D3DTS_VIEW);
    _matrix invView = XMMatrixInverse(nullptr, view);

    _vector camRight = XMVector3Normalize(invView.r[0]);
    _vector camForward = XMVector3Normalize(invView.r[2]);

    _vector hitDirWorld = XMVector3Normalize(XMVectorSetY(m_hitDirWorld, 0.f));

    float dotF = XMVectorGetX(XMVector3Dot(hitDirWorld, camForward));
    float dotR = XMVectorGetX(XMVector3Dot(hitDirWorld, camRight));

    float dirX = dotR;
    float dirY = -dotF;

    float len = sqrtf(dirX * dirX + dirY * dirY);
    if (len < 1e-4f)
    {
        tf->SetState(MATRIXROW_POSITION, s_basePos);
        tf->RotateRadian(_vector{ 0.f, 0.f, 1.f, 0.f }, 0.f);
        return;
    }

    dirX /= len;
    dirY /= len;
    float angle = -atan2f(dirX, -dirY);
    tf->RotateRadian(_vector{ 0.f, 0.f, 1.f, 0.f }, angle);

    const _float edgeOffset = 40.f;
    _float2 centerPos = { g_iWinSizeX >> 1, g_iWinSizeY >> 1 };
    UI_DESC desc = hitDir->GetUIDesc();
    desc.x = centerPos.x + dirX * edgeOffset;
    desc.y = centerPos.y + dirY * edgeOffset;
    hitDir->ApplyUIDesc(desc);
}

Player* Player::Create()
{
    Player* pInstance = new Player();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Player");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Container* Player::Clone(void* pArg)
{
    Player* pInstance = new Player(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Player");
        SafeRelease(pInstance);
    }

    return pInstance;
}
    
void Player::Free()
{
    __super::Free();
}
