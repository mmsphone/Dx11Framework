#include "Player.h"

#include "EngineUtility.h"

#include "Weapon_AR.h"
#include "SMG_Projectile.h"
#include "Layer.h"
#include "UI.h"
#include "UILabel.h"
#include "UIImage.h"
#include "endingUI.h"
#include "Grenade.h"
#include "BloodHitEffect.h"
#include "Drone.h"
#include "Worm.h"
#include "Shieldbug.h"

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
    pDesc->fSpeedPerSec = 3.5f;
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
    pPhysics->SetOnGround(false);
    m_isPlayingMinigame = false;

    return S_OK;
}

void Player::Update(_float fTimeDelta)
{
    InputCheck();

    UpdateAfkSound(fTimeDelta);

    if (m_kbActive)
    {
        HitBack(fTimeDelta);
    }

    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (pInfo != nullptr)
        pInfo->Update(fTimeDelta);

    if (auto pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine"))))
        pSM->Update(fTimeDelta);

    __super::Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    _vector vPos = pTransform->GetState(MATRIXROW_POSITION);
    m_pEngineUtility->SetActiveLightsByDistance(vPos, 40.f);
    m_pEngineUtility->SetActiveShadowLightsByDistance(vPos, 20.f);

    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

    if (auto* pAtkCol = static_cast<Collision*>(FindComponent(TEXT("AttackCollision"))))
        pAtkCol->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));


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
    Collision* pAttackCollision = dynamic_cast<Collision*>(FindComponent(TEXT("AttackCollision")));
    pAttackCollision->Render();
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

    _float r = m_pEngineUtility->Random(0, 7);
    if(r >= 6)
        m_pEngineUtility->PlaySound2D("FBX_playerHit1", 0.7f);
    else if (r >= 5)
        m_pEngineUtility->PlaySound2D("FBX_playerHit2", 0.7f);
    else if (r >= 4)
        m_pEngineUtility->PlaySound2D("FBX_playerHit3", 0.7f);
    else if (r >= 3)
        m_pEngineUtility->PlaySound2D("FBX_playerHit4", 0.7f);
    else if (r >= 2)
        m_pEngineUtility->PlaySound2D("FBX_playerHit5", 0.7f);
    else if (r >= 1)
        m_pEngineUtility->PlaySound2D("FBX_playerHit6", 0.7f);
    else
        m_pEngineUtility->PlaySound2D("FBX_playerHit7", 0.7f);
}

void Player::SetPlayingMinigame(_bool bPlayingMinigame)
{
    m_isPlayingMinigame = bPlayingMinigame;
}

void Player::SetPickupState()
{
    m_isPickup = true;
}

void Player::SetGlobalSightScale(const _float& sightScale)
{
    m_globalSightScale = sightScale;
}

_float Player::GetGlobalSightScale() const
{
    return m_globalSightScale;
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

    //본체
    {
        CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
        OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
        XMStoreFloat3(&OBBDesc.vExtents, _vector{ 0.25f, 1.f, 0.25f } / scaleOffset);
        OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
        if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
            return E_FAIL;
    }

    //근접공격용
    {
        CollisionBoxOBB::COLLISIONOBB_DESC atk{};
        atk.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
        // 살짝 넓고 더 앞으로 — 필요하면 숫자 조절하면서 튜닝
        XMStoreFloat3(&atk.vExtents, _vector{ 0.25f, 1.f, 0.6f } / scaleOffset);
        atk.vCenter = _float3(0.f, atk.vExtents.y * 0.5f, -atk.vExtents.z * 0.6f);
        if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("AttackCollision"), nullptr, &atk)))
            return E_FAIL;
    }

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
        { "Attack",     3 },
        { "MeleeAttack",4 },
        { "Throw",      5 },
        { "Pickup",     6 },
        { "Reload",     7 },
    };
}

HRESULT Player::SetUpStateMachine()
{
    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pSM || !pModel)
        return E_FAIL;

    auto CanReload = [](Player* pPlayer) -> bool
        {
            if (!pPlayer) return false;
            if (pPlayer->m_Parts.empty()) return false;

            Object* pWeapon = pPlayer->m_Parts[0];
            if (!pWeapon) return false;

            Info* partInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
            if (!partInfo) return false;

            INFO_DESC desc = partInfo->GetInfo();
            _int curBullet = *std::get_if<_int>(desc.GetPtr("CurBullet"));
            _int maxBullet = *std::get_if<_int>(desc.GetPtr("MaxBullet"));
            _int curAmmo = *std::get_if<_int>(desc.GetPtr("CurAmmo"));

            // 남은 탄약 없으면 불가
            if (curAmmo <= 0)
                return false;

            // 이미 만탄이면 재장전 의미 없음
            if (curBullet >= maxBullet)
                return false;

            return true;
    };

    auto CanShoot = [](Player* pPlayer) -> bool
        {
            if (!pPlayer) return false;
            if (pPlayer->m_Parts.empty()) return false;

            Object* pWeapon = pPlayer->m_Parts[0];
            if (!pWeapon) return false;

            Info* partInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
            if (!partInfo) return false;

            INFO_DESC desc = partInfo->GetInfo();
            _int curBullet = *std::get_if<_int>(desc.GetPtr("CurBullet"));
            return curBullet > 0;
        };

    auto CanThrow = [](Player* pPlayer) -> bool
        {
            if (!pPlayer) return false;

            Info* pInfo = static_cast<Info*>(pPlayer->FindComponent(TEXT("Info")));
            if (!pInfo) return false;

            INFO_DESC desc = pInfo->GetInfo();
            _int grenadeCount = *std::get_if<_int>(desc.GetPtr("GrenadeCount"));
            return grenadeCount > 0;
        };

    //상태 정의
    pSM->RegisterState("Idle", {
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pPlayer || !pModel)
                return;
            pModel->SetAnimation(pPlayer->FindAnimIndex("Idle"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
             Player* pPlayer = dynamic_cast<Player*>(owner);
             if (auto pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model"))))
                 pModel->PlayAnimation(fTimeDelta);
        },
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

            if (auto pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model"))))
                pModel->PlayAnimation(fTimeDelta * 1.3f);

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

            if (auto pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model"))))
                pModel->PlayAnimation(fTimeDelta * 1.3f);

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
                return;

            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
            pModel->SetAnimation(pPlayer->FindAnimIndex("Attack"), false, 0.05f, true);
            pPlayer->Shoot();

            _float r = EngineUtility::GetInstance()->Random(0, 2);
            if (r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_shootAR1", 0.7f);
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_shootAR2", 0.7f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer)
                return;

            if (auto pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model"))))
                pModel->PlayAnimation(fTimeDelta * 1.4f);

            pPlayer->Rotate(fTimeDelta);
            Object* pWeapon = pPlayer->m_Parts.at(0);
            if (pWeapon == nullptr)
                return;
        },
        nullptr
    });

    pSM->RegisterState("MeleeAttack", {
           // Enter
         [](Object* owner, StateMachine* sm) {
        auto* pPlayer = static_cast<Player*>(owner);
        if (!pPlayer) return;

        auto* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
        if (!pModel) return;

        // 근접 애니 한 번 재생
        pModel->SetAnimation(pPlayer->FindAnimIndex("MeleeAttack"), false, 0.05f, false);

        pPlayer->m_meleeDidHit = false;

        // 근접 공격 사운드
        EngineUtility::GetInstance()->PlaySound2D("FBX_swing", 0.7f);

        // ============================
        //  마우스 방향으로 회전 세팅
        //  (Shoot / Throw 와 동일한 로직 축약 버전)
        // ============================
        auto* util = EngineUtility::GetInstance();
        _float2 mouse = util->GetMousePos();
        float d01 = 1.f;
        if (util->ReadDepthAtPixel((int)mouse.x, (int)mouse.y, &d01))
        {
            D3D11_VIEWPORT vp{};
            UINT vpCount = 1;
            util->GetContext()->RSGetViewports(&vpCount, &vp);

            float sx = std::clamp(mouse.x, 0.0f, vp.Width - 1.0f) + 0.5f;
            float sy = std::clamp(mouse.y, 0.0f, vp.Height - 1.0f) + 0.5f;

            _matrix V = util->GetTransformMatrix(D3DTS_VIEW);
            _matrix P = util->GetTransformMatrix(D3DTS_PROJECTION);

            _vector screen = XMVectorSet(sx, sy, d01, 1.f);
            _vector pickPos = XMVector3Unproject(
                screen,
                vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
                0.0f, 1.0f,
                P, V, XMMatrixIdentity()
            );

            auto* tf = static_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
            if (tf)
            {
                _vector playerPos = tf->GetState(MATRIXROW_POSITION);
                _vector moveDir = pickPos - playerPos;
                _vector dirXZ = XMVector3Normalize(XMVectorSetY(moveDir, 0.f));

                if (!XMVector3Equal(dirXZ, XMVectorZero()))
                {
                    _vector look = XMVector3Normalize(XMVectorSetY(tf->GetState(MATRIXROW_LOOK), 0.f));
                    const float baseOffset = XM_PI;

                    float curYaw = atan2f(XMVectorGetX(look),  XMVectorGetZ(look));
                    float dstYaw = baseOffset + atan2f(
                        XMVectorGetX(dirXZ),
                        XMVectorGetZ(dirXZ)
                    );

                    float start = curYaw;
                    float target = dstYaw;
                    float delta = target - start;
                    while (delta > XM_PI) delta -= XM_2PI;
                    while (delta < -XM_PI) delta += XM_2PI;
                    target = start + delta;

                    if (fabsf(delta) < 1e-4f)
                    {
                        // 거의 정렬이면 바로 스냅
                        tf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, target);
                        pPlayer->m_shootAimActive = false;
                    }
                    else
                    {
                        // 수류탄/총과 동일하게 보간 회전
                        pPlayer->m_shootYawStart = start;
                        pPlayer->m_shootYawTarget = target;
                        pPlayer->m_shootAimElapsed = 0.f;
                        pPlayer->m_shootAimActive = true;
                    }
                }
            }
        }
    },
               // Update
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
        auto* pPlayer = static_cast<Player*>(owner);
        if (!pPlayer) return;
        
        auto* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
        if (!pModel) return;
        
        pModel->PlayAnimation(fTimeDelta * 1.8f);
        pPlayer->Rotate(fTimeDelta);

          // 애니 중간 타이밍에 한 번만 히트
        const float cur = pModel->GetCurAnimTrackPos();
        const float total = pModel->GetCurAnimDuration();
        if (!pPlayer->m_meleeDidHit && total > 0.f && cur > total * 0.35f && cur < total * 0.7f)
         {
        pPlayer->DoMeleeAttack();
        pPlayer->m_meleeDidHit = true;
        }
         },
               // Exit
        [](Object* owner, StateMachine* sm) {
        auto* pPlayer = static_cast<Player*>(owner);
        if (!pPlayer) return;
        
        pPlayer->m_meleeDidHit = false;
                   // 입력 플래그는 InputCheck에서 매 프레임 초기화되므로 여기서 손댈 필요 X
        }
     });
    pSM->RegisterState("Reload", {
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pPlayer->FindAnimIndex("Reload"), false, 0.05f, false);

            EngineUtility::GetInstance()->PlaySound2D("FBX_reload", 0.8f);

            _float r = EngineUtility::GetInstance()->Random(0, 2);
            if(r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerReload1");
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerReload2");

            r = EngineUtility::GetInstance()->Random(0, 3);
            if (r >= 2)
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerShell1", 0.5f);
            else if (r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerShell2", 0.5f);
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerShell3", 0.5f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = static_cast<Player*>(owner);

            if (auto pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model"))))
                pModel->PlayAnimation(fTimeDelta * 2.5f);


        },
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            Object* pWeapon = pPlayer->m_Parts.at(0);
            Info* partInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));

            INFO_DESC desc = partInfo->GetInfo();
            _int curBullet = *std::get_if<_int>(desc.GetPtr("CurBullet"));
            _int maxBullet = *std::get_if<_int>(desc.GetPtr("MaxBullet"));
            _int curAmmo = *std::get_if<_int>(desc.GetPtr("CurAmmo"));

            if (curBullet < maxBullet && curAmmo > 0)
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

    pSM->RegisterState("Pickup", {
        // Enter
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return;

            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel) return;

            // Pickup 애니메이션 1회 재생
            pModel->SetAnimation(pPlayer->FindAnimIndex("Pickup"), false, 0.05f, false);
        },
        // Update
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return;

            if (auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model"))))
                pModel->PlayAnimation(fTimeDelta * 2.f);
        },
        // Exit
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return;

            // 한 번 소비하고 플래그 리셋
            pPlayer->m_isPickup = false;
        }
        });

    pSM->RegisterState("Throw", {
        // Enter
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return;

            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel) return;

            pPlayer->BeginThrowAimFromMouse();
            // ThrowAway 애니메이션 재생
            pModel->SetAnimation(pPlayer->FindAnimIndex("Throw"), false, 0.05f, false);
            
            pPlayer->m_inThrowState = true;
            pPlayer->m_throwed = false;
            pPlayer->GetParts().at(0)->SetVisible(false);

            EngineUtility::GetInstance()->PlaySound2D("FBX_throw", 0.7f);

            _float r = EngineUtility::GetInstance()->Random(0, 2);
            if (r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerThrow1");
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_playerThrow2");
        },
        // Update
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(L"Model"));

            pModel->PlayAnimation(fTimeDelta * 2.5f);
            pPlayer->Rotate(fTimeDelta);
            if (pPlayer->m_throwed == false && pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.53)
            {
                pPlayer->m_throwed = true;
                pPlayer->ThrowGrenade();
            }
        },
        // Exit
        [](Object* owner, StateMachine* sm) {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return;

            // 플래그 소비
            pPlayer->m_inThrowState = false;
            pPlayer->m_isThrow = false;
            pPlayer->GetParts().at(0)->SetVisible(true);
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
        [CanShoot](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            if (!pPlayer->m_isShoot) 
                return false;
            return CanShoot(pPlayer);
        });
    pSM->AddTransition("Idle", "Throw", FindPriorityIndex("Throw"),
        [CanThrow](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            if (!pPlayer->m_isThrow)
                return false;

            return CanThrow(pPlayer);
        });
    pSM->AddTransition("Idle", "MeleeAttack", FindPriorityIndex("MeleeAttack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;
            return pPlayer->m_isMelee;
            });
    pSM->AddTransition("Idle", "Reload", FindPriorityIndex("Reload"),
        [CanReload](Engine::Object* owner, Engine::StateMachine*)
        {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return false;

            if (!pPlayer->m_isReload)
                return false;

            return CanReload(pPlayer);
        });
    pSM->AddTransition("Idle", "Pickup", FindPriorityIndex("Pickup"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;

            return pPlayer->m_isPickup;
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
        [CanShoot](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            if (!pPlayer->m_isShoot) 
                return false;
            return CanShoot(pPlayer);
        });
    pSM->AddTransition("Walk", "MeleeAttack", FindPriorityIndex("MeleeAttack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;
            return pPlayer->m_isMelee;
            });
    pSM->AddTransition("Walk", "Throw", FindPriorityIndex("Throw"),
        [CanThrow](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            if (!pPlayer->m_isThrow)
                return false;

            return CanThrow(pPlayer);
        });
    pSM->AddTransition("Walk", "Reload", FindPriorityIndex("Reload"),
        [CanReload](Engine::Object* owner, Engine::StateMachine*)
        {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return false;

            if (!pPlayer->m_isReload)
                return false;

            return CanReload(pPlayer);
        });
    pSM->AddTransition("Walk", "Pickup", FindPriorityIndex("Pickup"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;

            return pPlayer->m_isPickup;
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
        [CanShoot](Engine::Object* owner, Engine::StateMachine*) {
        Player* pPlayer = dynamic_cast<Player*>(owner);
        if (!pPlayer)
            return false;
        if (!pPlayer->m_isShoot) 
            return false;
        return CanShoot(pPlayer);
        });
    pSM->AddTransition("Run", "MeleeAttack", FindPriorityIndex("MeleeAttack"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;
            return pPlayer->m_isMelee;
            });
    pSM->AddTransition("Run", "Throw", FindPriorityIndex("Throw"),
        [CanThrow](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            if (!pPlayer->m_isThrow)
                return false;

            return CanThrow(pPlayer);
        });
    pSM->AddTransition("Run", "Reload", FindPriorityIndex("Reload"),
        [CanReload](Engine::Object* owner, Engine::StateMachine*)
        {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return false;

            if (!pPlayer->m_isReload)
                return false;

            return CanReload(pPlayer);
        });
    pSM->AddTransition("Run", "Pickup", FindPriorityIndex("Pickup"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;

            return pPlayer->m_isPickup;
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
        [CanShoot](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;

            if (!pModel->isAnimFinished()) return false;
            if (!pPlayer->m_isShoot) return false;
            return CanShoot(pPlayer);
        }, true);
    pSM->AddTransition("Attack", "Reload", FindPriorityIndex("Reload"),
        [CanReload](Engine::Object* owner, Engine::StateMachine*)
        {
            Player* pPlayer = static_cast<Player*>(owner);
            if (!pPlayer) return false;

            Model* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            if (!pModel->isAnimFinished())
                return false;

            if (!pPlayer->m_isReload)
                return false;

            return CanReload(pPlayer);
        });

    // MeleeAttack ->
    pSM->AddTransition("MeleeAttack", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*)
        {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            // 애니 끝났고, 이동 입력도 없으면 Idle
            return pModel->isAnimFinished() && !pPlayer->m_isMove;
        });

    pSM->AddTransition("MeleeAttack", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*)
        {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            // 애니 끝났고, 걷기 입력이면 Walk
            return pModel->isAnimFinished() && pPlayer->m_isMove && !pPlayer->m_isSprint;
        });

    pSM->AddTransition("MeleeAttack", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*)
        {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            // 애니 끝났고, 달리기 입력이면 Run
            return pModel->isAnimFinished() && pPlayer->m_isMove && pPlayer->m_isSprint;
        });

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
        [CanShoot](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = static_cast<Player*>(owner);
            Model* pModel = static_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel->isAnimFinished()) return false;
            if (!pPlayer->m_isShoot) return false;
            return CanShoot(pPlayer);
        });

    //Throw ->
    pSM->AddTransition("Throw", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            return pModel->isAnimFinished() && !pPlayer->m_isMove;
        });

    pSM->AddTransition("Throw", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            return pModel->isAnimFinished() && pPlayer->m_isMove && !pPlayer->m_isSprint;
        });

    pSM->AddTransition("Throw", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            Model* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            return pModel->isAnimFinished() && pPlayer->m_isMove && pPlayer->m_isSprint;
        });

    //Pickup ->
    pSM->AddTransition("Pickup", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            // 애니 끝나고 입력도 정지면 Idle
            return pModel->isAnimFinished() && !pPlayer->m_isMove;
        });

    pSM->AddTransition("Pickup", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            // 애니 끝나고 걷기 입력이면 Walk
            return pModel->isAnimFinished() && pPlayer->m_isMove && !pPlayer->m_isSprint;
        });

    pSM->AddTransition("Pickup", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            auto* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer) return false;

            auto* pModel = dynamic_cast<Model*>(pPlayer->FindComponent(TEXT("Model")));
            if (!pModel) return false;

            // 애니 끝나고 달리기 입력이면 Run
            return pModel->isAnimFinished() && pPlayer->m_isMove && pPlayer->m_isSprint;
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
    desc.SetData("MaxHP", _float{ 100.f });
    desc.SetData("CurHP", _float{ 100.f });
    desc.SetData("InvincibleTime", _float{ 0.3f });
    desc.SetData("InvincibleLeft", _float{ 0.f });
    desc.SetData("Time", _float{ 0.f });
    desc.SetData("LastHit", _float{ -999.f });
    desc.SetData("IsHit", _bool{ false });
    desc.SetData("Faction", FACTION_PLAYER);
    desc.SetData("GrenadeCount", _int{ 0 });
    desc.SetData("SpecialCount", _int{ 0 });
    desc.SetData("MeleeDamage", _float{ 10.f });
    
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
        desc.z = 0.15f;
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
        desc.z = 0.15f;
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
    m_isShoot = false;
    m_isSprint = false;
    m_isReload = false;
    m_isThrow = false;
    m_isMelee = false;

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
    m_isReload = m_pEngineUtility->IsKeyDown(DIK_R) ? true : false;
    m_isThrow = m_pEngineUtility->IsKeyDown(DIK_G) ? true : false;
    m_isMelee = m_pEngineUtility->IsKeyDown(DIK_F) ? true : false;

    if (m_pEngineUtility->IsKeyPressed(DIK_T))
    {
        Info* pInfo = static_cast<Info*>(FindComponent(L"Info"));
		_int iSpecialCount = *std::get_if<_int>(pInfo->GetInfo().GetPtr("SpecialCount"));
        if (iSpecialCount > 0)
        {
            --iSpecialCount;
			pInfo->GetInfo().SetData("SpecialCount", iSpecialCount);
            
			_float fCurHP = *std::get_if<_float>(pInfo->GetInfo().GetPtr("CurHP"));
			_float fMaxHP = *std::get_if<_float>(pInfo->GetInfo().GetPtr("MaxHP"));
			fCurHP += 50.f;
			fCurHP = fCurHP > fMaxHP ? fMaxHP : fCurHP;
            pInfo->GetInfo().SetData("CurHP", fCurHP);

            _float hpRatio = fCurHP / fMaxHP;
			static_cast<UIImage*>(m_pEngineUtility->FindUI(L"playerHpFront"))->SetHPRatio(hpRatio);
            static_cast<UILabel*>(m_pEngineUtility->FindUI(L"specialCount"))->SetText(L"x" + to_wstring(iSpecialCount));
        }
    }
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
    if (!m_isShoot && m_isMove && !m_inThrowState)
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
        _int curAmmo = *std::get_if<_int>(desc.GetPtr("CurAmmo"));

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

        if (curBullet <= 0 && curAmmo > 0)
        {
            if (auto* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine"))))
            {
                pSM->SetState("Reload");
            }
        }
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

void Player::BeginThrowAimFromMouse()
{
    m_hasThrowAim = false;
    m_throwAimDir = XMVectorZero();

    // 1) 마우스 → 뎁스 읽기
    _float2 mouse = m_pEngineUtility->GetMousePos();
    float d01 = 1.f;
    if (!m_pEngineUtility->ReadDepthAtPixel((int)mouse.x, (int)mouse.y, &d01))
        return;

    // 2) Viewport / Unproject
    D3D11_VIEWPORT vp{};
    UINT vpCount = 1;
    m_pEngineUtility->GetContext()->RSGetViewports(&vpCount, &vp);

    float sx = std::clamp(mouse.x, 0.0f, vp.Width - 1.0f) + 0.5f;
    float sy = std::clamp(mouse.y, 0.0f, vp.Height - 1.0f) + 0.5f;

    _matrix V = m_pEngineUtility->GetTransformMatrix(D3DTS_VIEW);
    _matrix P = m_pEngineUtility->GetTransformMatrix(D3DTS_PROJECTION);

    _vector screen = XMVectorSet(sx, sy, d01, 1.f);
    _vector pickPos = XMVector3Unproject(
        screen,
        vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height,
        0.0f, 1.0f,
        P, V, XMMatrixIdentity()
    );

    Transform* pTf = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Model* pModel = static_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTf || !pModel)
        return;

    // 3) 왼손(수류탄) 위치 기준으로 방향 계산
    const _float4x4* pPartPosMatPtr = m_Parts.at(0)->GetCombinedWorldMatrix();
    _matrix partWorld = XMLoadFloat4x4(pPartPosMatPtr);

    const _float4x4* pHandSocketPtr =
        pModel->GetSocketBoneMatrixPtr("ValveBiped.Bip01_L_Hand");
    _matrix handLocal = XMLoadFloat4x4(pHandSocketPtr);

    _vector handPos = handLocal.r[3];
    handPos = XMVector3TransformCoord(handPos, partWorld);

    _vector moveDir = pickPos - handPos;
    _vector dirXZ = XMVector3Normalize(XMVectorSetY(moveDir, 0.f));
    if (XMVector3Equal(dirXZ, XMVectorZero()))
        return;

    // ★ 수류탄 던질 때 쓸 방향 저장
    m_throwAimDir = dirXZ;
    m_hasThrowAim = true;

    // 4) 플레이어 회전 보간 시작 (Shoot에서 하던 것과 동일한 방식)
    _vector look = XMVector3Normalize(XMVectorSetY(pTf->GetState(MATRIXROW_LOOK), 0.f));
    const float baseOffset = XM_PI;

    float curYaw = atan2f(XMVectorGetX(look), XMVectorGetZ(look));
    float dstYaw = baseOffset + atan2f(XMVectorGetX(dirXZ), XMVectorGetZ(dirXZ));

    float start = curYaw;
    float target = dstYaw;
    float delta = target - start;
    while (delta > XM_PI) delta -= XM_2PI;
    while (delta < -XM_PI) delta += XM_2PI;
    target = start + delta;

    if (fabsf(delta) < 1e-4f)
    {
        // 거의 이미 정렬되어 있으면 바로 스냅
        pTf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, target);
        m_shootAimActive = false;
    }
    else
    {
        // 보간 회전 시작
        m_shootYawStart = start;
        m_shootYawTarget = target;
        m_shootAimElapsed = 0.f;
        m_shootAimActive = true;
    }
}

void Player::ThrowGrenade()
{
    // 1) 수류탄 개수 체크
    Info* pInfo = static_cast<Info*>(FindComponent(L"Info"));
    if (!pInfo)
        return;

    INFO_DESC desc = pInfo->GetInfo();
    _int grenadeCount = *std::get_if<_int>(desc.GetPtr("GrenadeCount"));
    if (grenadeCount <= 0)
        return;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Model* pModel = static_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pModel)
        return;

    // 2) 방향: Enter에서 계산해 둔 에임 방향 우선 사용
    _vector dir = XMVectorZero();
    if (m_hasThrowAim)
    {
        dir = m_throwAimDir;
    }
    else
    {
        // 혹시 실패했으면 현재 바라보는 방향으로 fallback
        _vector look = -pTransform->GetState(MATRIXROW_LOOK);
        dir = XMVector3Normalize(XMVectorSetY(look, 0.f));
        if (XMVector3Equal(dir, XMVectorZero()))
            dir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    }

    // 3) 시작 위치: 왼손 소켓 기준
    const _float4x4* socket = pModel->GetSocketBoneMatrixPtr("ValveBiped.Bip01_L_Hand");
    _vector socketPos = _vector{ socket->_41, socket->_42, socket->_43, socket->_44 };

    _vector pos = pTransform->GetState(MATRIXROW_POSITION);
    _vector spawnPos = pos + socketPos * scaleOffset;

    // 4) GRENADE_DESC 채워서 AddObject
    GRENADE_DESC gDesc{};
    XMStoreFloat3(&gDesc.vStartPos, spawnPos);
    XMStoreFloat3(&gDesc.vDir, dir);

    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    if (FAILED(m_pEngineUtility->AddObject(sceneId, TEXT("Grenade"), sceneId, TEXT("Projectile"), &gDesc)))
        return;

    // 5) 개수 감소 + UI 갱신
    --grenadeCount;
    desc.SetData("GrenadeCount", grenadeCount);
    pInfo->BindInfoDesc(desc);

    if (auto* ui = dynamic_cast<UILabel*>(m_pEngineUtility->FindUI(L"grenadeCount")))
        ui->SetText(to_wstring(grenadeCount));
}

void Player::DoMeleeAttack()
{
    // 1) 내 히트박스 / Info / Transform 가져오기
    auto* pAtkCol = static_cast<Collision*>(FindComponent(TEXT("AttackCollision")));
    if (!pAtkCol)
        return;

    auto* pInfo = static_cast<Info*>(FindComponent(TEXT("Info")));
    auto* pTf = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pInfo || !pTf)
        return;

    INFO_DESC& myDesc = pInfo->GetInfo();

    _float meleeDamage = 10.f;
    if (auto p = myDesc.GetPtr("MeleeDamage"))
        meleeDamage = *std::get_if<_float>(p);

    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();

    auto HitLayer = [&](const wchar_t* layerName)
        {
            Layer* pLayer = m_pEngineUtility->FindLayer(sceneId, layerName);
            if (!pLayer) return;

            list<Object*> objs = pLayer->GetAllObjects();
            for (auto* obj : objs)
            {
                if (!obj) continue;

                // Info 확인해서 아군은 무시
                auto* pTargetInfo = static_cast<Info*>(obj->FindComponent(TEXT("Info")));
                if (!pTargetInfo) continue;

                INFO_DESC& tDesc = pTargetInfo->GetInfo();
                if (auto pf = tDesc.GetPtr("Faction"))
                {
                    _int faction = *std::get_if<_int>(pf);
                    if (faction == FACTION_PLAYER) // 혹시라도 플레이어나 아군이면 스킵
                        continue;
                }

                auto* pTargetCol = static_cast<Collision*>(obj->FindComponent(TEXT("Collision")));
                if (!pTargetCol) continue;

                // ★ 실제 히트박스 교차 체크
                if (!pAtkCol->Intersect(pTargetCol))
                    continue;

                // 무적 시간 체크
                _float invLeft = 0.f;
                if (auto p = tDesc.GetPtr("InvincibleLeft"))
                    invLeft = *std::get_if<_float>(p);
                if (invLeft > 0.f)
                    continue;

                _float curHP = 0.f;
                _float invT = 0.1f;
                if (auto p = tDesc.GetPtr("CurHP"))          curHP = *std::get_if<_float>(p);
                if (auto p = tDesc.GetPtr("InvincibleTime")) invT = *std::get_if<_float>(p);

                curHP = max(0.f, curHP - meleeDamage);
                tDesc.SetData("CurHP", _float{ curHP });
                tDesc.SetData("IsHit", _bool{ true });
                tDesc.SetData("InvincibleLeft", _float{ invT });

                if (auto pT = tDesc.GetPtr("Time"))
                    tDesc.SetData("LastHit", *std::get_if<_float>(pT));

                pTargetInfo->BindInfoDesc(tDesc);

                // 피 이펙트
                if (auto* pTargetTf = static_cast<Transform*>(obj->FindComponent(TEXT("Transform"))))
                {
                    BloodHitEffect::BLOODHITEFFECT_DESC e{};
                    e.fLifeTime = 0.25f;
                    e.bLoop = false;
                    e.bAutoKill = true;
                    e.fRotationPerSec = 0.f;
                    e.fSpeedPerSec = 0.f;
                    XMStoreFloat3(&e.vCenterWS, pTargetTf->GetState(MATRIXROW_POSITION));
                    e.baseColor = _float4(0.2f, 1.0f, 0.2f, 1.f);

                    m_pEngineUtility->AddObject(
                        SCENE::GAMEPLAY,
                        TEXT("BloodHitEffect"),
                        SCENE::GAMEPLAY,
                        TEXT("Effect"),
                        &e
                    );
                }

                // 필요하면 여기서 넉백도 줄 수 있음 (몬스터 쪽에 SetHit 만들어져 있으면)
                // 예시:
                if (auto* drone = dynamic_cast<Drone*>(obj)) {
                    _vector dirKB = XMVector3Normalize(
                        XMVectorSetY(static_cast<Transform*>(drone->FindComponent(L"Transform"))->GetState(MATRIXROW_POSITION) - pTf->GetState(MATRIXROW_POSITION), 0.f));
                    drone->SetHit(dirKB, 3.0f, 0.2f);
                }
                else if (auto* worm = dynamic_cast<Worm*>(obj)) {
                    _vector dirKB = XMVector3Normalize(
                        XMVectorSetY(static_cast<Transform*>(worm->FindComponent(L"Transform"))->GetState(MATRIXROW_POSITION) - pTf->GetState(MATRIXROW_POSITION), 0.f));
                    worm->SetHit(dirKB, 3.0f, 0.2f);
                }
                else if (auto* shieldbug = dynamic_cast<Shieldbug*>(obj)) {
                    _vector dirKB = XMVector3Normalize(
                        XMVectorSetY(static_cast<Transform*>(shieldbug->FindComponent(L"Transform"))->GetState(MATRIXROW_POSITION) - pTf->GetState(MATRIXROW_POSITION), 0.f));
                    shieldbug->SetHit(dirKB, 3.0f, 0.2f);
                }
            }
        };

    // ★ 실제로 치고 싶은 몬스터 레이어들 나열
    HitLayer(TEXT("Drone"));
    HitLayer(TEXT("Worm"));
    HitLayer(TEXT("Shieldbug"));
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

    Info* pInfo = static_cast<Info*>(FindComponent(L"Info"));
    if (*std::get_if<_float>(pInfo->GetInfo().GetPtr("CurHP")) <= 0)
    {
        pInfo->GetInfo().SetData("InvincibleLeft", _float{ 999.f });
        static_cast<endingUI*>(EngineUtility::GetInstance()->FindUI(L"endingUI"))->SetEndingText("mission\nfailed", endingUI::ENDTYPE::END_LOSE);
        static_cast<endingUI*>(EngineUtility::GetInstance()->FindUI(L"endingUI"))->Show(true);
    }
}

void Player::UpdateAfkSound(_float fTimeDelta)
{
    const bool isActive =
        m_isMove ||
        m_isShoot ||
        m_isReload ||
        m_isThrow ||
        m_kbActive ||
        m_isPickup ||
        m_isPlayingMinigame;

    if (isActive)
    {
        // 하나라도 행동이 있으면 타이머 리셋
        m_idleTime = 0.f;
        m_afkSoundAcc = 0.f;
        return;
    }

    // 아무 행동도 없을 때만 idle 누적
    m_idleTime += fTimeDelta;

    // 10초 이전엔 아무 것도 안 함
    if (m_idleTime < 10.f)
    {
        m_afkSoundAcc = 0.f; // 10초 전까지는 쿨다운도 같이 리셋
        return;
    }

    // 10초를 넘겼으면 AFK 상태, 여기서부터 3초마다 사운드
    m_afkSoundAcc += fTimeDelta;
    if (m_afkSoundAcc >= 3.f)
    {
        _float r = m_pEngineUtility->Random(0, 3);
        if (r >= 2)
            m_pEngineUtility->PlaySound2D("FBX_playerAFK1");
        else if( r >= 1)
            m_pEngineUtility->PlaySound2D("FBX_playerAFK2");
        else
            m_pEngineUtility->PlaySound2D("FBX_playerAFK3");

        m_afkSoundAcc = 0.f;
    }
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
