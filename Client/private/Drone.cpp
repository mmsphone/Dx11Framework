#include "Drone.h"

#include "EngineUtility.h"

Drone::Drone()
    : ObjectTemplate{}
{
}

Drone::Drone(const Drone& Prototype)
    : ObjectTemplate{ Prototype }
{
}

HRESULT Drone::Initialize(void* pArg)
{
    OBJECT_DESC* pDesc = new OBJECT_DESC;

    pDesc->fSpeedPerSec = 3.f;
    pDesc->fRotationPerSec = XMConvertToRadians(180.0f);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    SetUpAnimIndexMap();
    SetUpPriorityIndexMap();
    
    if (FAILED(SetUpStateMachine()))
        return E_FAIL;

    SetUpAIController();

    return S_OK;
}

void Drone::Update(_float fTimeDelta)
{
    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (pAI != nullptr)
        pAI->Update(fTimeDelta);

    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    if (pSM != nullptr)
        pSM->Update(fTimeDelta);

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if(pModel != nullptr)
        pModel->PlayAnimation(fTimeDelta);

}

void Drone::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
    __super::LateUpdate(fTimeDelta);
}

HRESULT Drone::Render()
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
    if (nullptr == pLightDesc)
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4)))) return E_FAIL;

    _int bUseTex = 0;
    if (FAILED(pShader->BindRawValue("g_bUseTexture", &bUseTex, sizeof(_int)))) return E_FAIL;

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

_uint Drone::FindAnimIndex(string animName, _uint fallback) const
{
    auto it = m_animIndexMap.find(animName);
    if (it != m_animIndexMap.end())
        return it->second;
    return fallback;
}

_uint Drone::FindPriorityIndex(string toStateName, _uint fallback) const
{
    auto it = m_priorityIndexMap.find(toStateName);
    if (it != m_priorityIndexMap.end())
        return it->second;
    return fallback;
}

Drone* Drone::Create()
{
    Drone* pInstance = new Drone();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Drone");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Object* Drone::Clone(void* pArg)
{
    Drone* pInstance = new Drone(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Drone");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Drone::Free()
{
    __super::Free();
}

HRESULT Drone::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Drone"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("StateMachine"), TEXT("StateMachine"), nullptr, nullptr)))
        return E_FAIL;

    AIController::AICONTROLLER_DESC aiDesc{};
    aiDesc.sightRange = 6.f;
    aiDesc.fovDeg = 360.f;
    aiDesc.attackRange = 1.f;
    aiDesc.chaseKeepSec = 3.f;

    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("AIController"), TEXT("AIController"), nullptr, &aiDesc)))
        return E_FAIL;

    return S_OK;
}

void Drone::SetUpAnimIndexMap()
{
    m_animIndexMap = {
        { "Attack",     0 },
        { "Die",        1 },
        { "Hit",        2 },
        { "Idle",       3 },
        { "Roar",       4 },
        { "Walk",       5 },
    };
}

void Drone::SetUpPriorityIndexMap()
{
    m_priorityIndexMap = {
            { "Idle",       0 },
            { "Walk",       1 },
            { "Attack",     2 },
            { "Roar",       3 },
            { "Hit",        4 },
            { "Die",        5 },
    };
}

HRESULT Drone::SetUpStateMachine()
{
    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pSM || !pModel)
        return E_FAIL;

    //상태 정의
    pSM->RegisterState("Roar", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pDrone || !pModel)
                return;
            pModel->SetAnimation(pDrone->FindAnimIndex("Roar"), false, 0.05f);
        },
        nullptr,
        nullptr
        });

    pSM->RegisterState("Idle", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pDrone || !pModel)
                return;
            pModel->SetAnimation(pDrone->FindAnimIndex("Idle"), true, 0.1f);
        },
        nullptr,
        nullptr
        });

    pSM->RegisterState("Walk", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pDrone || !pModel)
                return;
            pModel->SetAnimation(pDrone->FindAnimIndex("Walk"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return;
            pDrone->Move(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Attack", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pDrone || !pModel)
                return;
            pModel->SetAnimation(pDrone->FindAnimIndex("Attack"), false, 0.05f);
        },
        nullptr,
        nullptr
        });


    //전이 등록
    //Roar ->
    pSM->AddTransition("Roar", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine* sm) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && sm->GetTimeInState() > 0.1f;
        });

    //Idle ->
    pSM->AddTransition("Idle", "Walk", FindPriorityIndex("Walk"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            return pDrone->m_isMove;
        });
    pSM->AddTransition("Idle", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            return false;
        });

    //Walk ->
    pSM->AddTransition("Walk", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            return !pDrone->m_isMove;
        });


    // 시작 상태: Roar
    pSM->SetState("Roar");

    return S_OK;
}

void Drone::SetUpAIController()
{
    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (pAI == nullptr)
        return;

    AIController::AI_OWNER_DESC owner{};

    owner.getOwnerPos = [this]() -> _vector {
        if (auto tr = dynamic_cast<Transform*>(FindComponent(TEXT("Transform"))))
            return tr->GetState(POSITION);
        return XMVectorZero();
        };
    owner.getOwnerLook = [this]() -> _vector {
        if (auto tr = dynamic_cast<Transform*>(FindComponent(TEXT("Transform"))))
            return XMVector3Normalize(tr->GetState(LOOK));
        return XMVectorSet(0.f, 0.f, 1.f, 0.f);
        };
    owner.applyInput = [this](bool move, _vector dir, bool sprint) {
        m_isMove = move;
        // 지면 투영 + 정규화
        dir = XMVectorSetY(dir, 0.f);
        m_moveDir = XMVector3Equal(dir, XMVectorZero()) ? XMVectorZero() : XMVector3Normalize(dir);
        m_isSprint = sprint;
        };
    // LOS 없으면 true로 처리(나중에 레이캐스트 붙이면 여기서 연결)
    owner.hasLineOfSight = [](_vector, _vector) { return true; };

    pAI->BindOwnerDesc(owner);
}

void Drone::Move(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform || !m_isMove) return;

    _vector prePos = pTransform->GetState(POSITION);

    float amp = m_isSprint ? 1.4f : 1.f;
    pTransform->Translate(m_moveDir, fTimeDelta * amp);

    _vector newPos = pTransform->GetState(POSITION);
    if (!m_pEngineUtility->IsInCell(newPos))
        pTransform->SetState(POSITION, prePos);

    // 진행 방향으로 회전(플레이어와 동일한 방식)
    _vector baseLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    _vector dir = XMVectorSetY(m_moveDir, 0.f);
    if (!XMVector3Equal(dir, XMVectorZero()))
    {
        float dot = std::clamp(XMVectorGetX(XMVector3Dot(baseLook, dir)), -1.0f, 1.0f);
        float ang = acosf(dot);
        _vector axis = XMVector3Cross(baseLook, dir);
        if (XMVectorGetY(axis) < 0.f) 
            ang = -ang;
        
        pTransform->RotateRadian(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f) + ang);
    }
}
