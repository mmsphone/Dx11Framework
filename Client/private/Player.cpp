#include "Player.h"

#include "EngineUtility.h"

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

    pDesc->iNumParts = 0;
    pDesc->fSpeedPerSec = 4.f;
    pDesc->fRotationPerSec = XMConvertToRadians(180.0f);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    SetUpAnimIndexMap();
    SetUpPriorityIndexMap();
    SetUpStateMachine();

    return S_OK;
}

void Player::Update(_float fTimeDelta)
{
    InputCheck();

    if (auto pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine"))))
        pSM->Update(fTimeDelta);

    if (auto pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model"))))
        pModel->PlayAnimation(fTimeDelta);

    __super::Update(fTimeDelta);

}

void Player::LateUpdate(_float fTimeDelta)
{
    // 🔹 렌더 그룹 등록
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);

    // 🔹 파츠 업데이트 (ContainerTemplate이 내부적으로 관리)
    __super::LateUpdate(fTimeDelta);
}

HRESULT Player::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));

    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW)))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4)))) return E_FAIL;

    const LIGHT_DESC* pLightDesc = m_pEngineUtility->GetLight(0);
    if (!pLightDesc) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4)))) return E_FAIL;

    _int bUseTex = 0;
    pShader->BindRawValue("g_bUseTexture", &bUseTex, sizeof(_int));

    _uint       iNumMeshes = pModel->GetNumMeshes();

    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (pModel->GetModelData()->bones.size() > 0)
        {
            if (FAILED(pModel->BindShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
                continue;

            pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
            pShader->Begin(0);
            pModel->Render(i);
        }
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

HRESULT Player::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Player"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("StateMachine"), TEXT("StateMachine"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT Player::SetUpParts()
{
    // 🔹 예시: 파츠 등록 (나중에 애니메이션/무기 교체에 대응 가능)
    //AddPart(TEXT("Body"), PlayerBody::Create());
    //AddPart(TEXT("Weapon"), PlayerWeapon::Create());

    return S_OK;
}

void Player::SetUpAnimIndexMap()
{
    m_animIndexMap = {
        { "Idle",       0 },
        { "Walk",       5 },
        { "Run",        4 },
        { "Shoot",      8 }
    };
}

void Player::SetUpPriorityIndexMap()
{
    m_priorityIndexMap = {
        { "Idle",       0 },
        { "Walk",       1 },
        { "Run",        2 },
        { "Shoot",      3 }
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
        },
        nullptr
    });

    pSM->RegisterState("Shoot", {
        [](Object* owner, StateMachine* sm){
            Player* pPlayer = dynamic_cast<Player*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pPlayer || !pModel)
                return;
            pModel->SetAnimation(pPlayer->FindAnimIndex("Shoot"), true, 0.05f);
        },
        nullptr,
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

    pSM->AddTransition("Idle", "Shoot", FindPriorityIndex("Shoot"),
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

    pSM->AddTransition("Walk", "Shoot", FindPriorityIndex("Shoot"),
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
    
    pSM->AddTransition("Run", "Shoot", FindPriorityIndex("Shoot"),
        [](Engine::Object* owner, Engine::StateMachine*) {
        Player* pPlayer = dynamic_cast<Player*>(owner);
        if (!pPlayer)
            return false;
        return pPlayer->m_isShoot;
        });

    //Shoot ->
    pSM->AddTransition("Shoot", "Idle", FindPriorityIndex("Idle"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return !pPlayer->m_isShoot && !pPlayer->m_isMove;
        });

    pSM->AddTransition("Shoot", "Walk", FindPriorityIndex("Walk"),
        [](Engine::Object* owner, Engine::StateMachine*) {
            Player* pPlayer = dynamic_cast<Player*>(owner);
            if (!pPlayer)
                return false;
            return !pPlayer->m_isShoot && pPlayer->m_isMove;
        });

    pSM->AddTransition("Shoot", "Run", FindPriorityIndex("Run"),
        [](Engine::Object* owner, Engine::StateMachine*) {
        Player* pPlayer = dynamic_cast<Player*>(owner);
        if (!pPlayer)
            return false;
        return !pPlayer->m_isShoot && pPlayer->m_isMove && pPlayer->m_isSprint; 
        });

    //초기 상태 설정
    pSM->SetState("Idle");

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
        m_moveDir = dir;
    }
    else
    {
        m_isMove = true;
        m_moveDir = XMVector3Normalize(dir);
    }

    m_isSprint = m_pEngineUtility->IsKeyDown(DIK_LSHIFT) ? true : false;
    m_isShoot = m_pEngineUtility->IsMouseDown(LB) ? true : false;

}

void Player::Move(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;

    if (m_isMove == false)
        return;

    //이동
    _vector prePos = pTransform->GetState(POSITION);

    _float moveAmp = 1.f;
    if (m_isSprint) moveAmp = 1.4f;
    pTransform->Translate(m_moveDir, fTimeDelta * moveAmp);

    _vector movePos = pTransform->GetState(POSITION);
    if (m_pEngineUtility->IsInCell(movePos) == false)
    {
        pTransform->SetState(POSITION, prePos);
    }

    //회전
    _vector baseLook = _vector{ 0.f,0.f,1.f,0.f };
    baseLook = XMVectorSetY(baseLook, 0.f);
    m_moveDir = XMVectorSetY(m_moveDir, 0.f);
    
    float dot = XMVectorGetX(XMVector3Dot(baseLook, m_moveDir));
    dot = std::clamp(dot, -1.0f, 1.0f);
    float angle = acosf(dot);
    
    _vector axis = XMVector3Cross(baseLook, m_moveDir);
    if (XMVectorGetY(axis) < 0.f)
        angle = -angle;

    pTransform->RotateRadian(_vector{ 0.f, 1.f, 0.f, 0.f }, XMConvertToRadians(180.f) + angle);
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
