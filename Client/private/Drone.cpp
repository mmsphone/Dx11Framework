#include "Drone.h"

#include "EngineUtility.h"

#include "Player.h"
#include "BloodHitEffect.h"

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

    SafeDelete(pDesc);

    SetUpAnimIndexMap();
    SetUpPriorityIndexMap();
    
    if (FAILED(SetUpStateMachine()))
        return E_FAIL;

    if (FAILED(SetUpInfo()))
        return E_FAIL;

    if (FAILED(SetUpAIProcess()))
        return E_FAIL;

    if(m_pAIInputCache == nullptr)
        m_pAIInputCache = new AIINPUT_DESC{};

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetScale(scaleOffset, scaleOffset, scaleOffset);
    
    return S_OK;
}

void Drone::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (pAI != nullptr)
    {
        SetUpAIInputData();
        pAI->SetInput(*m_pAIInputCache);
        pAI->Update(fTimeDelta);
    }

    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (pInfo != nullptr)
        pInfo->Update(fTimeDelta);

    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    if (pSM != nullptr)
        pSM->Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
    Collision* pAttackCollision = static_cast<Collision*>(FindComponent(TEXT("AttackCollision")));
    pAttackCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
}

void Drone::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
    {
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_SHADOWLIGHT, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Drone::Render()
{
    Transform* playerTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!playerTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(playerTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW)))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION)))) return E_FAIL;
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

HRESULT Drone::RenderShadow(_uint iIndex)
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
        pShader->Begin(iIndex + 1);
        pModel->Render(i);
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

    SafeDelete(m_pAIInputCache);
}

HRESULT Drone::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Drone"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("StateMachine"), TEXT("StateMachine"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("AIController"), TEXT("AIController"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Info"), TEXT("Info"), nullptr, nullptr)))
        return E_FAIL;

    {
        CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
        OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
        XMStoreFloat3(&OBBDesc.vExtents, _vector{ 0.5f, 0.5f, 0.5f } / scaleOffset);
        OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
        if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
            return E_FAIL;
    }
    {
        CollisionBoxOBB::COLLISIONOBB_DESC atk{};
        atk.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
        XMStoreFloat3(&atk.vExtents, _vector{ 0.5f, 0.5f, 0.6f } / scaleOffset);
        atk.vCenter = _float3(0.f, atk.vExtents.y * 0.5f, -atk.vExtents.z * 0.6f);
        if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("AttackCollision"), nullptr, &atk)))
            return E_FAIL;
    }


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

    //상태 등록
    pSM->RegisterState("Roar", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pDrone->FindAnimIndex("Roar"), false, 0.05f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Idle", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pDrone->FindAnimIndex("Idle"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Walk", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pDrone->FindAnimIndex("Walk"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pDrone->Move(fTimeDelta);
            pDrone->Rotate(fTimeDelta);
            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Attack", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pDrone->FindAnimIndex("Attack"), false, 0.05f, true);
            pDrone->m_targetAttackable = true;
        },
        [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.6f &&
                pModel->GetCurAnimTrackPos() < pModel->GetCurAnimDuration() * 0.8f )
            {
                pDrone->Attack();
                pDrone->m_targetAttackable = false;
            }
            else if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.2f)
            {
                pDrone->Rotate(fTimeDelta);
            }
            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Hit", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pDrone->FindAnimIndex("Hit"), false, 0.05f, true);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Die", {
        [](Object* owner, StateMachine* sm) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            Info* pInfo = static_cast<Info*>(owner->FindComponent(TEXT("Info")));

            pModel->SetAnimation(pDrone->FindAnimIndex("Die"), false, 0.05f);
            INFO_DESC desc = pInfo->GetInfo();
            desc.SetData("InvincibleLeft", _float{ 10.f });
            pInfo->BindInfoDesc(desc);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            
            pModel->PlayAnimation(fTimeDelta);
            if (pModel->isAnimFinished())
                owner->SetDead(true);
        },
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
            return pDrone->m_isAttack;
        });
    pSM->AddTransition("Idle", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            if (*std::get_if<_bool>(pInfo->GetInfo().GetPtr("IsHit")))
            {
                pInfo->GetInfo().SetData("IsHit", false);
                return true;
            }
            else
                return false;
        });
    pSM->AddTransition("Idle", "Die", FindPriorityIndex("Die"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false; 
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Walk ->
    pSM->AddTransition("Walk", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            return !pDrone->m_isMove;
        });
    pSM->AddTransition("Walk", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            return pDrone->m_isAttack;
        });
    pSM->AddTransition("Walk", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            if (*std::get_if<_bool>(pInfo->GetInfo().GetPtr("IsHit")))
            {
                pInfo->GetInfo().SetData("IsHit", false);
                return true;
            }
            else
                return false;
        });
    pSM->AddTransition("Walk", "Die", FindPriorityIndex("Die"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Attack ->
    pSM->AddTransition("Attack", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pDrone->m_isAttack && !pDrone->m_isMove;
        });
    pSM->AddTransition("Attack", "Walk", FindPriorityIndex("Walk"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pDrone->m_isAttack && pDrone->m_isMove;
        });
    pSM->AddTransition("Attack", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() &&pDrone->m_isAttack;
        }, true);
    pSM->AddTransition("Attack", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            if (*std::get_if<_bool>(pInfo->GetInfo().GetPtr("IsHit")))
            {
                pInfo->GetInfo().SetData("IsHit", false);
                return true;
            }
            else
                return false;
        });
    pSM->AddTransition("Attack", "Die", FindPriorityIndex("Die"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Hit ->
    pSM->AddTransition("Hit", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pDrone->m_isAttack && !pDrone->m_isMove;
        });
    pSM->AddTransition("Hit", "Walk", FindPriorityIndex("Walk"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pDrone->m_isAttack && pDrone->m_isMove;
        });
    pSM->AddTransition("Hit", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && pDrone->m_isAttack;
        }, true);
    pSM->AddTransition("Hit", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            if (*std::get_if<_bool>(pInfo->GetInfo().GetPtr("IsHit")))
            {
                pInfo->GetInfo().SetData("IsHit", false);
                return true;
            }
            else
                return false;
        }, true);
    pSM->AddTransition("Hit", "Die", FindPriorityIndex("Die"),
        [](Object* owner, StateMachine*) {
            Drone* pDrone = dynamic_cast<Drone*>(owner);
            if (!pDrone)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pDrone->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    // 시작 상태: Roar
    pSM->SetState("Roar");

    return S_OK;
}

HRESULT Drone::SetUpInfo()
{
    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (!pInfo)
        return E_FAIL;

    INFO_DESC desc;
    desc.SetData("MaxHP", _float{ 120.f });
    desc.SetData("CurHP", _float{ 120.f });
    desc.SetData("InvincibleTime", _float{ 0.1f });
    desc.SetData("InvincibleLeft", _float{ 0.f });
    desc.SetData("Time", _float{ 0.f });
    desc.SetData("LastHit", _float{ -999.f });
    desc.SetData("IsHit", _bool{ false }); 
    desc.SetData("Faction", FACTION_MONSTER);
    desc.SetData("AttackDamage", _float{ 15.f });
    pInfo->BindInfoDesc(desc);

    return S_OK;
}

void Drone::SetUpAIInputData()
{
    Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), TEXT("Player"), 0);
    if (!pPlayer)
        return;
    Transform* pTransform = static_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));

    m_pAIInputCache->SetData("PlayerPos", pTransform->GetState(MATRIXROW_POSITION));

    m_pAIInputCache->SetData("SightRange", _float{ 10.f });
    m_pAIInputCache->SetData("FovDegree", _float{ 360.f });
    m_pAIInputCache->SetData("AttackRange", _float{ 1.2f });
    m_pAIInputCache->SetData("ChasePersistTime", _float{ 5.f });
}

HRESULT Drone::SetUpAIProcess()
{
    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (!pAI) 
        return E_FAIL;

    AIPROCESS_DESC proc{};

    // [1] SENSE: 입력 보강만 담당 (this에서 Transform 조회)
    proc.sense = [this](AIINPUT_DESC& in, _float /*dt*/, _float time)
        {
            Transform* tf = dynamic_cast<Transform*>(this->FindComponent(TEXT("Transform")));

            // DronePos
            if (tf) in.SetData("DronePos", tf->GetState(MATRIXROW_POSITION));

            const AIValue* pDronePos = in.GetPtr("DronePos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");

            // 기본 가시성 false
            in.SetData("Visible", _bool{ false });

            if (pDronePos && pPlayerPos)
            {
                const _vector dronePos = std::get<_vector>(*pDronePos);
                const _vector playerPos = std::get<_vector>(*pPlayerPos);

                // 거리
                const _float distance = XMVectorGetX(XMVector3Length(playerPos - dronePos));
                in.SetData("Distance", _float{ distance });

                // 파라미터(없으면 기본값)
                const float sightRange = in.GetPtr("SightRange") ? std::get<_float>(*in.GetPtr("SightRange")) : 6.f;
                const float fovDeg = in.GetPtr("FovDegree") ? std::get<_float>(*in.GetPtr("FovDegree")) : 360.f;
                const float chasePersistTime = in.GetPtr("ChasePersistTime") ? std::get<_float>(*in.GetPtr("ChasePersistTime")) : 3.f;

                // 가시성 (XZ 평면 FOV)
                bool inRangeSight = (distance <= sightRange);

                bool inFov = true;
                if (fovDeg < 359.f && tf)
                {
                    _vector look = tf->GetState(MATRIXROW_LOOK);
                    _vector toT = XMVector3Normalize(playerPos - dronePos);
                    look = XMVector3Normalize(XMVectorSet(XMVectorGetX(look), 0.f, XMVectorGetZ(look), 0.f));
                    toT = XMVector3Normalize(XMVectorSet(XMVectorGetX(toT), 0.f, XMVectorGetZ(toT), 0.f));
                    const float cosang = std::clamp(XMVectorGetX(XMVector3Dot(look, toT)), -1.f, 1.f);
                    const float angDeg = XMConvertToDegrees(acosf(cosang));
                    inFov = (angDeg <= (fovDeg * 0.5f));
                }

                const bool visible = (inRangeSight && inFov);
                in.SetData("Visible", _bool{ visible });

                if (visible) in.SetData("LastSeenSec", _float{ time });

                float lastSeen = -1e9f;
                if (auto p = in.GetPtr("LastSeenSec")) lastSeen = std::get<_float>(*p);
                const bool chaseAlive = (time - lastSeen) <= chasePersistTime;
                in.SetData("ChaseAlive", _bool{ chaseAlive });
            }
        };

    // [2] DECIDE: 파생 판단(out: inRange/tracking/isMove/attack)
    proc.decide = [this](const AIINPUT_DESC& in, AIOUTPUT_DESC& out, _float /*dt*/, _float /*time*/)
        {
            const AIValue* pDronePos = in.GetPtr("DronePos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");

            out.SetData("inRange", _bool{ false });
            out.SetData("tracking", _bool{ false });
            out.SetData("isMove", _bool{ false });
            out.SetData("attack", _float{ 0.f });

            if (!pDronePos || !pPlayerPos) return;

            const _vector dronePos = std::get<_vector>(*pDronePos);
            const _vector playerPos = std::get<_vector>(*pPlayerPos);
            _vector dir = XMVector3Normalize(playerPos - dronePos);

            const float distance = in.GetPtr("Distance") ? std::get<_float>(*in.GetPtr("Distance")) : 1e9f;
            const bool  visible = in.GetPtr("Visible") ? std::get<_bool>(*in.GetPtr("Visible")) : false;
            const bool  chaseAlive = in.GetPtr("ChaseAlive") ? std::get<_bool>(*in.GetPtr("ChaseAlive")) : false;
            const float atkRange = in.GetPtr("AttackRange") ? std::get<_float>(*in.GetPtr("AttackRange")) : 1.f;

            const bool inRange = (distance <= atkRange);
            const bool tracking = (visible || chaseAlive);

            out.SetData("inRange", _bool{ inRange });
            out.SetData("tracking", _bool{ tracking });

            if (tracking && !inRange) {
                out.SetData("isMove", _bool{ true });
                out.SetData("moveDir", dir);
                out.SetData("sprint", _bool{ true });
            }
            if (tracking && inRange) {
                out.SetData("isAttack", _bool{ true });
            }
        };

    // [3] ACT: 출력 보정
    proc.act = [this](AIOUTPUT_DESC& out, _float /*dt*/, _float /*time*/)
        {
            if (auto p = out.GetPtr("moveDir")) {
                _vector dir = std::get<_vector>(*p);
                const float len = XMVectorGetX(XMVector3Length(dir));
                if (len > 1e-6f) out.SetData("moveDir", XMVectorScale(dir, 1.f / len));
            }
        };

    // [4] APPLY: Drone 멤버 반영
    proc.applyOutput = [this](const AIOUTPUT_DESC& out)
        {
            m_isMove = false;
            m_isSprint = false;
            m_isAttack = false;
            m_aimDir = XMVectorZero();

            if (auto p = out.GetPtr("isMove"))   m_isMove = std::get<_bool>(*p);
            if (auto p = out.GetPtr("sprint"))   m_isSprint = std::get<_bool>(*p);
            if (auto p = out.GetPtr("moveDir"))  m_aimDir = std::get<_vector>(*p);
            if (auto p = out.GetPtr("isAttack"))   m_isAttack = std::get<_bool>(*p);
        };

    pAI->BindProcessDesc(proc);
    return S_OK;
}

void Drone::Move(_float fTimeDelta)
{
    Transform* playerTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!playerTransform || !m_isMove) 
        return;

    _vector prePos = playerTransform->GetState(MATRIXROW_POSITION);

    float amp = m_isSprint ? 1.4f : 1.f;
    playerTransform->Translate(m_aimDir, fTimeDelta * amp);

    _vector newPos = playerTransform->GetState(MATRIXROW_POSITION);
    if (!m_pEngineUtility->IsInCell(newPos))
        playerTransform->SetState(MATRIXROW_POSITION, prePos);

}

void Drone::Rotate(_float fTimeDelta)
{
    Transform* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!tf) return;

    // 1) 목표 방향: 우선 m_aimDir, 없으면 플레이어 향하도록 계산
    _vector desiredDir = m_aimDir;
    if (XMVector3Equal(desiredDir, XMVectorZero()))
    {
        const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
        if (Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0))
            if (Transform* pPT = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform"))))
                desiredDir = XMVector3Normalize(XMVectorSetY(pPT->GetState(MATRIXROW_POSITION) - tf->GetState(MATRIXROW_POSITION), 0.f));
    }
    if (XMVector3Equal(desiredDir, XMVectorZero()))
        return;

    desiredDir = XMVector3Normalize(desiredDir);

    // 2) 현재/목표 yaw (모델 전방이 -Z라면 +PI 보정)
    const float baseOffset = XM_PI;

    _vector curLook = XMVector3Normalize(XMVectorSetY(tf->GetState(MATRIXROW_LOOK), 0.f));
    float curYaw = atan2f(XMVectorGetX(curLook), XMVectorGetZ(curLook));                 // [-PI,PI]
    float dstYaw = baseOffset + atan2f(XMVectorGetX(desiredDir), XMVectorGetZ(desiredDir));
    while (dstYaw > XM_PI) dstYaw -= XM_2PI;
    while (dstYaw < -XM_PI) dstYaw += XM_2PI;

    // 3) 목표가 의미 있게 바뀌면 재보간 시작
    float shortestDelta = dstYaw - m_yawTarget;
    while (shortestDelta > XM_PI) shortestDelta -= XM_2PI;
    while (shortestDelta < -XM_PI) shortestDelta += XM_2PI;

    const float reTargetEps = XMConvertToRadians(2.0f);
    if (!m_yawInterpActive || fabsf(shortestDelta) > reTargetEps)
    {
        m_yawInterpActive = true;
        m_yawInterpT = 0.f;
        m_yawStart = curYaw;
        m_yawTarget = dstYaw;
    }

    // 4) 보간 진행
    if (m_yawInterpActive)
    {
        m_yawInterpT += max(0.f, fTimeDelta);
        float t = std::clamp(m_yawInterpT / max(1e-4f, m_yawInterpDur), 0.f, 1.f);
        t = std::clamp(t, 0.f, 1.f);
        const float s = t * t * (3.f - 2.f * t);

        float shortestDelta = m_yawTarget - m_yawStart;
        while (shortestDelta > XM_PI) shortestDelta -= XM_2PI;
        while (shortestDelta < -XM_PI) shortestDelta += XM_2PI;

        float yawNow = m_yawStart + shortestDelta * s;
        while (yawNow > XM_PI) yawNow -= XM_2PI;
        while (yawNow < -XM_PI) yawNow += XM_2PI;

        // 절대 yaw를 세팅하는 방식(너의 Transform이 플레이어 코드와 동일 동작 가정)
        tf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, yawNow);

        if (t >= 1.f) m_yawInterpActive = false;
    }
}

void Drone::Attack()
{
    // 플레이어 찾기
    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
    if (!pPlayer) 
        return;
    Transform* pPlayerTransform = static_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
    Info* pPlayerInfo = static_cast<Info*>(pPlayer->FindComponent(TEXT("Info")));

    Collision* pAtkCol = static_cast<Collision*>(FindComponent(TEXT("AttackCollision")));
    Collision* pPlayerCol = static_cast<Collision*>(pPlayer->FindComponent(TEXT("Collision")));
    if (!pAtkCol->Intersect(pPlayerCol))
        return;

    Transform* pDroneTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Info* pDroneInfo = static_cast<Info*>(FindComponent(TEXT("Info")));
    _float damage = *std::get_if<_float>(pDroneInfo->GetInfo().GetPtr("AttackDamage"));

    if (ApplyDamageToPlayer(pPlayer, pPlayerInfo, damage))
    {
        if (Player* player = dynamic_cast<Player*>(pPlayer))
        {
            _vector dirKB = XMVector3Normalize(
                XMVectorSetY(pPlayerTransform->GetState(MATRIXROW_POSITION) - pDroneTransform->GetState(MATRIXROW_POSITION), 0.f)
            );
            const float kbPower = 3.0f;
            const float kbTime = 0.2f;
            player->SetHit(dirKB, kbPower, kbTime);
        }
    }
}

_bool Drone::ApplyDamageToPlayer(Object* pTarget, Info* info, _float damage)
{
    INFO_DESC Desc = INFO_DESC{};
    Desc = info->GetInfo();

    _float invLeft = 0.f;
    if (auto p = Desc.GetPtr("InvincibleLeft")) invLeft = *std::get_if<_float>(p);
    if (invLeft > 0.f) 
        return false;

    _float cur = 0.f, invT = 0.15f;
    if (auto p = Desc.GetPtr("CurHP"))            cur = *std::get_if<_float>(p);
    if (auto p = Desc.GetPtr("InvincibleTime"))   invT = *std::get_if<_float>(p);

    cur = max(0.f, cur - damage);
    Desc.SetData("CurHP", _float{ cur });
    Desc.SetData("IsHit", _bool{ true });
    Desc.SetData("InvincibleLeft", _float{ invT });

    if (auto pT = Desc.GetPtr("Time"))
        Desc.SetData("LastHit", *std::get_if<_float>(pT));

    info->BindInfoDesc(Desc);

    // ---- 여기서 히트 이펙트 생성 (Worm_Projectile과 동일 패턴) ----
    if (Transform* pTF = static_cast<Transform*>(pTarget->FindComponent(TEXT("Transform"))))
    {
        BloodHitEffect::BLOODHITEFFECT_DESC e{};
        e.fLifeTime = 0.25f;
        e.bLoop = false;
        e.bAutoKill = true;
        e.fRotationPerSec = 0.f;
        e.fSpeedPerSec = 0.f;
        XMStoreFloat3(&e.vCenterWS, pTF->GetState(MATRIXROW_POSITION));

        // 색은 적당히 빨간색 계열
        e.baseColor = _float4(1.0f, 0.2f, 0.2f, 1.f);

        m_pEngineUtility->AddObject(
            SCENE::GAMEPLAY,
            TEXT("BloodHitEffect"),
            SCENE::GAMEPLAY,
            TEXT("Effect"),
            &e
        );
    }

    return true;
}
