#include "Player.h"

#include "EngineUtility.h"

#include "Weapon_AR.h"
#include "SMG_Projectile.h"
#include "Layer.h"

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

    return S_OK;
}

void Player::Update(_float fTimeDelta)
{
    InputCheck();

    if (m_kbActive)
    {
        HitBack(fTimeDelta);
    }

    if (auto pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine"))))
        pSM->Update(fTimeDelta);

    if (auto pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model"))))
        pModel->PlayAnimation(fTimeDelta);

    __super::Update(fTimeDelta);
}

void Player::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this); 
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::SHADOWLIGHT, this);

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

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetShadowTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW, iIndex)))) 
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetShadowTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION, iIndex))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_ShadowLightFarDistance", m_pEngineUtility->GetShadowLightFarDistancePtr(iIndex), sizeof(_float))))
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
        { "Idle",       0 },
        { "Walk",       5 },
        { "Run",        4 },
        { "Attack",      8 },
    };
}

void Player::SetUpPriorityIndexMap()
{
    m_priorityIndexMap = {
        { "Idle",       0 },
        { "Walk",       1 },
        { "Run",        2 },
        { "Attack",      3 },
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
        [](Object* owner, StateMachine* sm, _float fDeltaTime) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return;
            pPlayer->Move(fDeltaTime);
            pPlayer->Rotate(fDeltaTime);
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
        [](Object* owner, StateMachine* sm, _float fDeltaTime) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return;
            pPlayer->Move(fDeltaTime);
            pPlayer->Rotate(fDeltaTime);
        },
        nullptr
    });

    pSM->RegisterState("Attack", {
        [](Object* owner, StateMachine* sm){
            Player* pPlayer = dynamic_cast<Player*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pPlayer || !pModel)
                return;
            pModel->SetAnimation(pPlayer->FindAnimIndex("Attack"), false, 0.05f, true);
            pPlayer->Attack();
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
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
    pInfo->BindInfoDesc(desc);

    return S_OK;
}

void Player::InputCheck()
{
    m_isMove = false;

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
    m_isShoot = m_pEngineUtility->IsMouseDown(LB) ? true : false;

}

void Player::Move(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform == nullptr || m_isMove == false)
        return;

    //이동
    _vector prePos = pTransform->GetState(POSITION);

    _float moveAmp = 1.f;
    if (m_isSprint) 
        moveAmp = 1.4f;
    pTransform->Translate(m_aimDir, fTimeDelta * moveAmp);

    const _vector movePos = pTransform->GetState(POSITION);
    if (m_pEngineUtility->IsInCell(movePos) == false)
    {
        pTransform->SetState(POSITION, prePos);
    }
    else
    {
        _vector fixedPos{};
        m_pEngineUtility->SetHeightOnCell(pTransform->GetState(POSITION), &fixedPos);
        pTransform->SetState(POSITION, fixedPos);
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
        _vector look = XMVector3Normalize(XMVectorSetY(tf->GetState(LOOK), 0.f));
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

void Player::Attack()
{
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

        Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
        if (pTransform == nullptr)
            return;
        _vector playerPos = pTransform->GetState(POSITION);
        _vector moveDir = pickPos - playerPos;

        Projectile::PROJECTILE_DESC Desc{};
        Desc.moveDir = moveDir;
        Desc.accTime = 0.f;
        Desc.damageAmount = 20.f;
        Desc.faction = FACTION_PLAYER;
        Desc.hitRadius = 1.f;
        Desc.lifeTime = 2.f;
        Desc.fSpeedPerSec = 20.f;

        _uint iCurSceneId = m_pEngineUtility->GetCurrentSceneId();
        if (FAILED(m_pEngineUtility->AddObject(iCurSceneId, TEXT("SMG_Projectile"), iCurSceneId, TEXT("Projectile"), &Desc)))
            return;

        const _float4x4* pPartPosMatPtr = m_Parts.at(0)->GetCombinedWorldMatrix();        
        _matrix pPartPosMat = XMLoadFloat4x4(pPartPosMatPtr);
        const _float4x4* pMuzzlePosMatPtr = static_cast<Model*>(m_Parts.at(0)->FindComponent(TEXT("Model")))->GetSocketBoneMatrixPtr("muzzle");
        _matrix pMuzzlePosMat = XMLoadFloat4x4(pMuzzlePosMatPtr);
        
        _vector muzzlePos = pMuzzlePosMat.r[3];
        muzzlePos = XMVector3TransformCoord(muzzlePos, pPartPosMat);

        static_cast<Transform*>(m_pEngineUtility->FindLayer(iCurSceneId, TEXT("Projectile"))->GetAllObjects().back()->FindComponent(TEXT("Transform")))->SetState(POSITION, muzzlePos);
        {
            Transform* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
            if (tf)
            {
                // 목표 방향: 수평화 후 정규화
                _vector dir = XMVector3Normalize(XMVectorSetY(moveDir, 0.f));
                if (!XMVector3Equal(dir, XMVectorZero()))
                {
                    // 현재/목표 yaw (+PI 오프셋: 모델이 -Z를 바라보는 정렬 보정 그대로 유지)
                    _vector look = XMVector3Normalize(XMVectorSetY(tf->GetState(LOOK), 0.f));
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

    auto* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (tf)
    {
        // 선형 감쇠(ease-out): 남은 시간 비율
        const float dt = max(0.f, fTimeDelta);
        const float t = (m_kbDuration > 1e-6f) ? (m_kbRemain / m_kbDuration) : 0.f; // 1->0
        const float speed = m_kbPower * t; // 남을수록 감속

        // 충돌/네비 체크
        _vector prePos = tf->GetState(POSITION);
        tf->Translate(m_kbDir, speed * dt);

        _vector newPos = tf->GetState(POSITION);
        if (!m_pEngineUtility->IsInCell(newPos))
            tf->SetState(POSITION, prePos);

        m_kbRemain -= dt;
        if (m_kbRemain <= 0.f)
            m_kbActive = false;
    }
    else
    {
        m_kbActive = false;
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
