#include "Shieldbug.h"

#include "EngineUtility.h"

#include "Player.h"
#include "BloodHitEffect.h"
#include "Layer.h"
#include "BloodDieEffect.h"

Shieldbug::Shieldbug() 
    : ObjectTemplate{} 
{
}

Shieldbug::Shieldbug(const Shieldbug& Prototype) 
    : ObjectTemplate{ Prototype } 
{
}

HRESULT Shieldbug::Initialize(void* pArg)
{
    OBJECT_DESC* pDesc = new OBJECT_DESC;

    m_speed = 3.f;
    pDesc->fSpeedPerSec = m_speed;
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

    if (m_pAIInputCache == nullptr)
        m_pAIInputCache = new AIINPUT_DESC{};

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetScale(scaleOffset, scaleOffset, scaleOffset);

    m_arriveRadius = 1.f;
    m_repathInterval = 1.f;
    m_repathTimer = m_pEngineUtility->Random(0.f, m_repathInterval);

    m_isDying = false;
    m_deathFade = 1.f;

    return S_OK;
}

void Shieldbug::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_kbActive)
        HitBack(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
        return;

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

    UpdatePath(fTimeDelta);

    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
    Collision* pAttackCollision = static_cast<Collision*>(FindComponent(TEXT("AttackCollision")));
    pAttackCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
}

void Shieldbug::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
    {
        if (!m_isDying)
        {
            m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
            m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_SHADOWLIGHT, this);
        }
        else
            m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_BLEND, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Shieldbug::Render()
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = static_cast<Model*>(FindComponent(TEXT("Model")));

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix"))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW)))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float)))) return E_FAIL;

    _float alphaMul = 1.f;
    if (m_isDying)
    {
        m_pEngineUtility->BindRenderTargetShaderResource(L"RenderTarget_Shade", pShader, "g_ShadeTexture");
        m_pEngineUtility->BindRenderTargetShaderResource(L"RenderTarget_Specular", pShader, "g_SpecularTexture");
        alphaMul = m_deathFade;
    }
    if (FAILED(pShader->BindRawValue("g_AlphaMul", &alphaMul, sizeof(_float))))
        return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (pModel->GetModelData()->bones.size() > 0)
        {
            if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
                continue;

            pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
            if (!m_isDying)
                pShader->Begin(0);   // 기존 디퍼드용 pass
            else
                pShader->Begin(5);
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

HRESULT Shieldbug::RenderShadow(_uint iIndex)
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

_uint Shieldbug::FindAnimIndex(string animName, _uint fallback) const
{
    auto it = m_animIndexMap.find(animName);
    if (it != m_animIndexMap.end())
        return it->second;
    return fallback;
}

_uint Shieldbug::FindPriorityIndex(string toStateName, _uint fallback) const
{
    auto it = m_priorityIndexMap.find(toStateName);
    if (it != m_priorityIndexMap.end())
        return it->second;
    return fallback;
}

void Shieldbug::SetHit(_vector dirXZ, float power, float duration)
{
    _vector dir = XMVector3Normalize(XMVectorSetY(dirXZ, 0.f));
    if (XMVector3Equal(dir, XMVectorZero()))
        return;

    m_kbDir = dir;
    m_kbPower = max(0.f, power);
    m_kbDuration = max(0.01f, duration);
    m_kbRemain = m_kbDuration;
    m_kbActive = true;
}

Shieldbug* Shieldbug::Create()
{
    Shieldbug* pInstance = new Shieldbug();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Shieldbug");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Object* Shieldbug::Clone(void* pArg)
{
    Shieldbug* pInstance = new Shieldbug(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Shieldbug");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Shieldbug::Free()
{
    __super::Free();

    SafeDelete(m_pAIInputCache);
}

HRESULT Shieldbug::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Shieldbug"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("StateMachine"), TEXT("StateMachine"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("AIController"), TEXT("AIController"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Info"), TEXT("Info"), nullptr, nullptr)))
        return E_FAIL;

    {
        CollisionBoxOBB::COLLISIONOBB_DESC OBBDesc{};
        OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
        XMStoreFloat3(&OBBDesc.vExtents, _vector{ 1.f, 1.f, 1.f } / scaleOffset);
        OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
        if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
            return E_FAIL;
    }
    {
        CollisionBoxOBB::COLLISIONOBB_DESC atk{};
        atk.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
        XMStoreFloat3(&atk.vExtents, _vector{ 0.8f, 0.5f, 0.8f } / scaleOffset);
        atk.vCenter = _float3(0.f, atk.vExtents.y * 0.5f, -atk.vExtents.z * 3.f);

        if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("AttackCollision"), nullptr, &atk)))
            return E_FAIL;
    }

    return S_OK;
}

void Shieldbug::SetUpAnimIndexMap()
{
    m_animIndexMap = {
        { "Die",        0 },
        { "Walk",       3 },
        { "Attack",     4 },
        { "Hit",        6 },
        { "Defend",     8 },
        { "Idle",       10 },
        { "Roar",       12 },
    };
}

void Shieldbug::SetUpPriorityIndexMap()
{
    m_priorityIndexMap = {
        { "Idle",       0 },
        { "Walk",       1 },
        { "Defend",     2 },
        { "Attack",     3 },
        { "Roar",       4 },
        { "Hit",        5 },
        { "Die",        6 },
    };
}

HRESULT Shieldbug::SetUpStateMachine()
{
    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pSM || !pModel)
        return E_FAIL;

    //상태 등록
    pSM->RegisterState("Roar", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Roar"), false, 0.05f);

            EngineUtility::GetInstance()->PlaySound2D("FBX_shieldbugRoar1", 0.7f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Idle", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Idle"), true, 0.1f);
        },
         [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Walk", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Walk"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pShieldbug->Move(fTimeDelta);
            pShieldbug->Rotate(fTimeDelta);
            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Defend", {
        [](Object* owner, StateMachine* /*sm*/) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Defend"), true, 0.15f);

            if (Info* info = dynamic_cast<Info*>(owner->FindComponent(TEXT("Info")))) {
                INFO_DESC desc = info->GetInfo();
                desc.SetData("IsHit", _bool{ false });
                info->BindInfoDesc(desc);
            }
            pShieldbug->m_isEnterDefend = true;
        },
        [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Shieldbug* pShieldbug = static_cast<Shieldbug*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (pModel) 
                pModel->PlayAnimation(fTimeDelta);

            if (auto* info = dynamic_cast<Info*>(owner->FindComponent(TEXT("Info")))) {
                auto desc = info->GetInfo();
                const float refill = 0.1f;
                desc.SetData("InvincibleLeft", _float{ refill });
                info->BindInfoDesc(desc);
            }

            if (pShieldbug->m_isEnterDefend)
            {
                if (pModel->GetCurAnimTrackPos() < pModel->GetCurAnimDuration() * 0.5f)
                    pShieldbug->Rotate(fTimeDelta);
                else
                    pShieldbug->m_isEnterDefend = false;
            }
        },
        [](Object* owner, StateMachine* /*sm*/) {
            if (auto* info = dynamic_cast<Info*>(owner->FindComponent(TEXT("Info")))) {
                auto desc = info->GetInfo();
                desc.SetData("InvincibleLeft", _float{ 0.f });
                info->BindInfoDesc(desc);
            }
        }
        });

    pSM->RegisterState("Attack", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = static_cast<Shieldbug*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Attack"), false, 0.05f, true);
            pShieldbug->m_targetAttackable = true;
        },
        [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.6f &&
                pModel->GetCurAnimTrackPos() < pModel->GetCurAnimDuration() * 0.8f)
            {
                pShieldbug->Attack();
                pShieldbug->m_targetAttackable = false;
            }
            else if (pModel->GetCurAnimTrackPos() < pModel->GetCurAnimDuration() * 0.2f)
            {
                pShieldbug->Rotate(fTimeDelta);
            }
            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Hit", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pShieldbug || !pModel)
                return;
            pModel->SetAnimation(pShieldbug->FindAnimIndex("Hit"), false, 0.05f, true);

            _float r = EngineUtility::GetInstance()->Random(0, 2);
            if (r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit1", 0.7f);
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit2", 0.7f);
        },
         [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Die", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = static_cast<Shieldbug*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Die"), false, 0.05f);

            Info* pInfo = static_cast<Info*>(owner->FindComponent(TEXT("Info")));
            INFO_DESC desc = pInfo->GetInfo();
            desc.SetData("InvincibleLeft", _float{ 10.f });
            pInfo->BindInfoDesc(desc);

            Collision* pCol = static_cast<Collision*>(owner->FindComponent(TEXT("Collision")));

            pShieldbug->m_isDying = true;
            pShieldbug->m_deathFade = 1.f;
            _float3 centerPos = static_cast<BoundingOrientedBox*>(pCol->GetWorldCollisionBox(COLLISIONTYPE_OBB))->Center;
            BloodDieEffect::BLOODDIEEFFECT_DESC e{};
            e.fLifeTime = 1.f;
            e.bLoop = false;
            e.bAutoKill = true;
            e.fRotationPerSec = 0.f;
            e.fSpeedPerSec = 0.f;
            e.vCenterWS = centerPos;
            e.baseColor = _float4(0.2f, 1.f, 0.2f, 1.f);
            EngineUtility::GetInstance()->AddObject( SCENE::GAMEPLAY, TEXT("BloodDieEffect"), SCENE::GAMEPLAY, TEXT("Effect"), &e);

            {
                Object* pPlayer = EngineUtility::GetInstance()->FindObject(SCENE::GAMEPLAY, L"Player", 0);
                QUEST_EVENT ev{};
                ev.type = EVENTTYPE_KILL;
                ev.pInstigator = pPlayer;
                ev.pTarget = owner;
                ev.tag = L"Shieldbug";
                EngineUtility::GetInstance()->PushEvent(ev);
            }
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            _float dieAnimFaster = 3.f;        
            pModel->PlayAnimation(fTimeDelta * dieAnimFaster);

            _float pos = pModel->GetCurAnimTrackPos();
            _float dur = pModel->GetCurAnimDuration();
            _float t = 0.f;
            if (dur > 1e-4f)
                t = clamp(pos / dur, 0.f, 1.f);
            _float eased = t * t;
            pShieldbug->m_deathFade = 1.f - eased;

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
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return pShieldbug->m_isMove;
        });     
    pSM->AddTransition("Idle", "Defend", FindPriorityIndex("Defend"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return pShieldbug->m_isDefend;
        });
    pSM->AddTransition("Idle", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return pShieldbug->m_isAttack;
        });
    pSM->AddTransition("Idle", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
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
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Walk ->
    pSM->AddTransition("Walk", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            return !pShieldbug->m_isMove;
        });
    pSM->AddTransition("Walk", "Defend", FindPriorityIndex("Defend"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return pShieldbug->m_isDefend;
        });
    pSM->AddTransition("Walk", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            return pShieldbug->m_isAttack;
        });
    pSM->AddTransition("Walk", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
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
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Defend ->
    pSM->AddTransition("Defend", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return !pShieldbug->m_isDefend && !pShieldbug->m_isMove && !pShieldbug->m_isAttack;
        });
    pSM->AddTransition("Defend", "Walk", FindPriorityIndex("Walk"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return pShieldbug->m_isMove && !pShieldbug->m_isDefend;
        });
    pSM->AddTransition("Defend", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            return pShieldbug->m_isAttack;
        });
    pSM->AddTransition("Defend", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
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
    pSM->AddTransition("Defend", "Die", FindPriorityIndex("Die"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug) return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
            if (!pInfo) return false;
            return pInfo->IsDead();
        });

    //Attack ->
    pSM->AddTransition("Attack", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pShieldbug->m_isAttack && !pShieldbug->m_isMove;
        });
    pSM->AddTransition("Attack", "Walk", FindPriorityIndex("Walk"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pShieldbug->m_isAttack && pShieldbug->m_isMove;
        });
    pSM->AddTransition("Attack", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && pShieldbug->m_isAttack;
        }, true);
    pSM->AddTransition("Attack", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
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
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Hit ->
    pSM->AddTransition("Hit", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pShieldbug->m_isAttack && !pShieldbug->m_isMove;
        });
    pSM->AddTransition("Hit", "Walk", FindPriorityIndex("Walk"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pShieldbug->m_isAttack && pShieldbug->m_isMove;
        });
    pSM->AddTransition("Hit", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && pShieldbug->m_isAttack;
        }, true);
    pSM->AddTransition("Hit", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            if ( *std::get_if<_bool>(pInfo->GetInfo().GetPtr("IsHit")))
            {
                pInfo->GetInfo().SetData("IsHit", false);
                return true;
            }
            else
                return false;
        }, true);
    pSM->AddTransition("Hit", "Die", FindPriorityIndex("Die"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pShieldbug->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    // 시작 상태: Roar
    pSM->SetState("Roar");

    return S_OK;
}

HRESULT Shieldbug::SetUpInfo()
{
    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (!pInfo)
        return E_FAIL;

    INFO_DESC desc;
    desc.SetData("MaxHP", _float{ 50.f });
    desc.SetData("CurHP", _float{ 50.f });
    desc.SetData("InvincibleTime", _float{ 0.1f });
    desc.SetData("InvincibleLeft", _float{ 0.f });
    desc.SetData("Time", _float{ 0.f });
    desc.SetData("LastHit", _float{ -999.f });
    desc.SetData("IsHit", _bool{ false });
    desc.SetData("Faction", FACTION_MONSTER);
    desc.SetData("AttackDamage", _float{ 30.f });
    pInfo->BindInfoDesc(desc);

    return S_OK;
}

void Shieldbug::SetUpAIInputData()
{
    Object* pPlayerPos = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), TEXT("Player"), 0);
    if (!pPlayerPos)
        return;
    Transform* pTransform = dynamic_cast<Transform*>(pPlayerPos->FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;
    m_pAIInputCache->SetData("PlayerPos", pTransform->GetState(MATRIXROW_POSITION));

    m_pAIInputCache->SetData("SightRange", _float{ 8.f });
    m_pAIInputCache->SetData("TrackRange", _float{ 4.f });
    m_pAIInputCache->SetData("FovDegree", _float{ 360.f });
    m_pAIInputCache->SetData("AttackRange", _float{ 2.5f });
    m_pAIInputCache->SetData("ChasePersistTime", _float{ 5.f });
}

HRESULT Shieldbug::SetUpAIProcess()
{
    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (!pAI)
        return E_FAIL;

    AIPROCESS_DESC proc{};

    // sense : 입력 데이터 보강
    proc.sense = [this](AIINPUT_DESC& in, _float fTimeDelta, _float time)
        {
            Transform* pTransform = dynamic_cast<Transform*>(this->FindComponent(TEXT("Transform")));

            if (pTransform) 
                in.SetData("ShieldbugPos", pTransform->GetState(MATRIXROW_POSITION));

            const AIValue* pShieldbugPos = in.GetPtr("ShieldbugPos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");

            in.SetData("Visible", _bool{ false });

            if (pShieldbugPos && pPlayerPos)
            {
                const _vector shieldbugPos = std::get<_vector>(*pShieldbugPos);
                const _vector playerPos = std::get<_vector>(*pPlayerPos);

                const _float distance = XMVectorGetX(XMVector3Length(playerPos - shieldbugPos));
                in.SetData("Distance", _float{ distance });

                float sightRange = in.GetPtr("SightRange") ? std::get<_float>(*in.GetPtr("SightRange")) : 10.f;
                _float globalSightScale = static_cast<Player*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0))->GetGlobalSightScale();
                sightRange *= globalSightScale;
                const float fovDeg = in.GetPtr("FovDegree") ? std::get<_float>(*in.GetPtr("FovDegree")) : 360.f;
                const float chasePersistTime = in.GetPtr("ChasePersistTime") ? std::get<_float>(*in.GetPtr("ChasePersistTime")) : 3.f;

                bool inRangeSight = (distance <= sightRange);

                bool inFov = true;
                if (fovDeg < 359.f && pTransform)
                {
                    _vector look = pTransform->GetState(MATRIXROW_LOOK);
                    _vector toT = XMVector3Normalize(playerPos - shieldbugPos);
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

    // decide : 상태 판단
    proc.decide = [this](const AIINPUT_DESC& in, AIOUTPUT_DESC& out, _float fTimeDelta, _float time)
        {
            out.SetData("isMove", _bool{ false });
            out.SetData("isAttack", _bool{ false });
            out.SetData("isDefend", _bool{ false });

            const AIValue* pShieldbugPos = in.GetPtr("ShieldbugPos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");
            if (!pShieldbugPos || !pPlayerPos)
                return;

            const _vector shieldbugPos = std::get<_vector>(*pShieldbugPos);
            const _vector playerPos = std::get<_vector>(*pPlayerPos);
            _vector dir = XMVector3Normalize(playerPos - shieldbugPos);

            const float distance = in.GetPtr("Distance") ? std::get<_float>(*in.GetPtr("Distance")) : 1e9f;
            const bool  visible = in.GetPtr("Visible") ? std::get<_bool>(*in.GetPtr("Visible")) : false;
            const bool  chaseAlive = in.GetPtr("ChaseAlive") ? std::get<_bool>(*in.GetPtr("ChaseAlive")) : false;

            const float attackRange = in.GetPtr("AttackRange") ? std::get<_float>(*in.GetPtr("AttackRange")) : 2.5f;

            // ★ 방어 거리 구간
            const float defendNear = 4.f;
            const float defendFar = 6.f;

            const bool inSight = visible;
            const bool inAttackRange = (distance <= attackRange);
            const bool inDefendBand = (distance >= defendNear && distance <= defendFar);

            // --- 우선순위: 공격 > 방어 > 추적 ---

            // 1) 공격 거리 안
            if ((inSight || chaseAlive) && inAttackRange)
            {
                out.SetData("isAttack", _bool{ true });
                return;
            }

            // 2) 4 ~ 6 구간: 방어 상태
            if (inSight && inDefendBand)
            {
                out.SetData("isDefend", _bool{ true });
                return;
            }

            // 3) 그 외(공격 거리 밖 & 방어 구간 밖)인데,
            //    시야 안에 있거나 추적 유지 시간 안이면 계속 쫓아감
            if ((inSight || chaseAlive) && !inAttackRange && !inDefendBand)
            {
                out.SetData("isMove", _bool{ true });
                out.SetData("moveDir", dir);
                return;
            }

            // 4) 나머지: 아무 것도 하지 않음 (Idle로 떨어지게)
        };

    // [3] ACT: 출력 보정
    proc.act = [this](AIOUTPUT_DESC& out, _float /*fTimeDelta*/, _float /*time*/)
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
            m_isAttack = false;
            m_aimDir = XMVectorZero();

            if (auto p = out.GetPtr("isMove"))   m_isMove = std::get<_bool>(*p);
            if (auto p = out.GetPtr("isAttack"))  m_isAttack = std::get<_bool>(*p);
            if (auto p = out.GetPtr("isDefend"))  m_isDefend = std::get<_bool>(*p);
            if (auto p = out.GetPtr("moveDir"))  m_aimDir = std::get<_vector>(*p);
        };

    pAI->BindProcessDesc(proc);
    return S_OK;
}

void Shieldbug::Move(_float fTimeDelta)
{
    if (!m_isMove)
        return;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    _vector prePos = pTransform->GetState(MATRIXROW_POSITION);

    // 현재 위치
    _float3 curPos3;
    XMStoreFloat3(&curPos3, prePos);

    // -----------------------------
    // [0] 같은 셀이면 → 플레이어 쪽으로 바로 이동
    // -----------------------------
    _bool   directToPlayer = false;
    _float3 directTarget3{};

    {
        const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
        Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);

        if (pPlayer)
        {
            Transform* pPlayerTf = static_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
            if (pPlayerTf)
            {
                _vector vPlayerPos = pPlayerTf->GetState(MATRIXROW_POSITION);
                _float3 playerPos3;
                XMStoreFloat3(&playerPos3, vPlayerPos);

                _int myCell = -1;
                _int playerCell = -1;

                if (m_pEngineUtility->IsInCell(prePos, &myCell) &&
                    m_pEngineUtility->IsInCell(vPlayerPos, &playerCell) &&
                    myCell >= 0 && myCell == playerCell)
                {
                    // 같은 네비 셀에 있으면 그냥 플레이어를 목표로
                    directToPlayer = true;
                    directTarget3 = playerPos3;
                }
            }
        }
    }

    // -----------------------------
    // [1] 목표점 계산 (직접 추적 vs 경로 추적)
    // -----------------------------
    _float3 target{};

    if (directToPlayer)
    {
        target = directTarget3;
    }
    else
    {
        // 경로가 없으면 이동 X
        if (m_path.empty() || m_pathIndex < 0 || m_pathIndex >= (_int)m_path.size())
            return;

        target = m_path[m_pathIndex];

        // ---- look-ahead: 앞 1~2개 웨이포인트까지 보면서 코너를 깎기 ----
        auto Dist3 = [](const _float3& a, const _float3& b)
            {
                float dx = a.x - b.x;
                float dy = a.y - b.y;
                float dz = a.z - b.z;
                return sqrtf(dx * dx + dy * dy + dz * dz);
            };

        float distToCur = Dist3(curPos3, target);

        const float lookAheadRadius1 = 4.0f;
        const float lookAheadRadius2 = 6.0f;
        const float lookAheadRadius3 = 8.0f;
        const float cutStrength1 = 0.5f;
        const float cutStrength2 = 0.4f;
        const float cutStrength3 = 0.3f;
        // 1칸 앞
        if (m_pathIndex + 1 < (_int)m_path.size() && distToCur < lookAheadRadius1)
        {
            const _float3& next1 = m_path[m_pathIndex + 1];

            float t = 1.0f - (distToCur / lookAheadRadius1);
            t = std::clamp(t, 0.0f, 1.0f);

            float blend = t * cutStrength1;

            _float3 blended{
                target.x * (1.0f - blend) + next1.x * blend,
                target.y * (1.0f - blend) + next1.y * blend,
                target.z * (1.0f - blend) + next1.z * blend
            };

            target = blended;
        }

        // 2칸 앞
        if (m_pathIndex + 2 < (_int)m_path.size() && distToCur < lookAheadRadius2)
        {
            const _float3& next2 = m_path[m_pathIndex + 2];

            float t2 = 1.0f - (distToCur / lookAheadRadius2);
            t2 = std::clamp(t2, 0.0f, 1.0f);

            float blend2 = t2 * cutStrength2;

            _float3 blended2{
                target.x * (1.0f - blend2) + next2.x * blend2,
                target.y * (1.0f - blend2) + next2.y * blend2,
                target.z * (1.0f - blend2) + next2.z * blend2
            };

            target = blended2;
        }

        // 3칸 앞
        if (m_pathIndex + 3 < (_int)m_path.size() && distToCur < lookAheadRadius3)
        {
            const _float3& next3 = m_path[m_pathIndex + 3];

            float t3 = 1.0f - (distToCur / lookAheadRadius3);
            t3 = std::clamp(t3, 0.0f, 1.0f);

            float blend3 = t3 * cutStrength3;

            _float3 blended3{
                target.x * (1.0f - blend3) + next3.x * blend3,
                target.y * (1.0f - blend3) + next3.y * blend3,
                target.z * (1.0f - blend3) + next3.z * blend3
            };

            target = blended3;
        }
        // ---- look-ahead 끝 ----
    }

    // 목표까지 벡터/거리
    _float3 toTarget3{
        target.x - curPos3.x,
        target.y - curPos3.y,
        target.z - curPos3.z
    };

    float distToTarget = sqrtf(
        toTarget3.x * toTarget3.x +
        toTarget3.y * toTarget3.y +
        toTarget3.z * toTarget3.z
    );

    // directToPlayer 아닐 때만 웨이포인트 인덱스 갱신
    if (!directToPlayer)
    {
        if (distToTarget < m_arriveRadius)
        {
            ++m_pathIndex;

            if (m_pathIndex >= (_int)m_path.size())
                return;

            target = m_path[m_pathIndex];
            toTarget3.x = target.x - curPos3.x;
            toTarget3.y = target.y - curPos3.y;
            toTarget3.z = target.z - curPos3.z;

            distToTarget = sqrtf(
                toTarget3.x * toTarget3.x +
                toTarget3.y * toTarget3.y +
                toTarget3.z * toTarget3.z
            );

            if (distToTarget <= 0.f)
                return;
        }
    }

    // 방향 벡터
    _vector toTarget = XMLoadFloat3(&toTarget3);
    _vector dir = XMVector3Normalize(toTarget);

    m_aimDir = dir;

    _float speed = m_speed;
    _float distance = speed * fTimeDelta;
    if (distance <= 0.f)
        return;

    // 네비용 delta
    _vector delta = XMVectorScale(dir, distance);

    // 1차 이동 적용
    pTransform->Translate(m_aimDir, fTimeDelta);
    _vector movePos = pTransform->GetState(MATRIXROW_POSITION);

    // -----------------------------
    // [2] Door 충돌 처리 (Player와 동일 패턴)
    // -----------------------------
    {
        Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
        pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

        Layer* pDoorLayer = m_pEngineUtility->FindLayer(m_pEngineUtility->GetCurrentSceneId(), TEXT("Door"));
        if (pDoorLayer)
        {
            list<Object*> doors = pDoorLayer->GetAllObjects();
            for (auto& door : doors)
            {
                Collision* pDoorCol = static_cast<Collision*>(door->FindComponent(TEXT("Collision")));
                if (pDoorCol && pDoorCol->Intersect(pCollision))
                {
                    // 문에 박으면 원위치로 롤백
                    pTransform->SetState(MATRIXROW_POSITION, prePos);
                    return;
                }
            }
        }
    }

    // -----------------------------
    // [3] 네비게이션 슬라이드 처리
    // -----------------------------
    _int cellIndex = -1;

    // 새 위치가 셀 안이면 그대로 OK
    if (m_pEngineUtility->IsInCell(movePos, &cellIndex))
        return;

    // 이전 위치조차 셀 밖이면 그냥 롤백
    if (!m_pEngineUtility->IsInCell(prePos, &cellIndex))
    {
        pTransform->SetState(MATRIXROW_POSITION, prePos);
        return;
    }

    // 슬라이드 벡터 적용
    _vector slideDelta{};
    if (m_pEngineUtility->GetSlideVectorOnCell(prePos, delta, cellIndex, &slideDelta))
    {
        pTransform->SetState(MATRIXROW_POSITION, prePos + slideDelta);

        // 안전장치: 여전히 셀 밖이면 롤백
        _vector slidePos = pTransform->GetState(MATRIXROW_POSITION);
        if (!m_pEngineUtility->IsInCell(slidePos))
            pTransform->SetState(MATRIXROW_POSITION, prePos);
    }
    else
    {
        pTransform->SetState(MATRIXROW_POSITION, prePos);
    }
}

void Shieldbug::Rotate(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform) return;

    // 1) 목표 방향: 우선 m_aimDir, 없으면 플레이어 향하도록 계산
    _vector desiredDir = m_aimDir;
    if (XMVector3Equal(desiredDir, XMVectorZero()))
    {
        const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
        if (Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0))
            if (Transform* pPT = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform"))))
                desiredDir = XMVector3Normalize(XMVectorSetY(pPT->GetState(MATRIXROW_POSITION) - pTransform->GetState(MATRIXROW_POSITION), 0.f));
    }
    if (XMVector3Equal(desiredDir, XMVectorZero()))
        return;

    desiredDir = XMVector3Normalize(desiredDir);

    // 2) 현재/목표 yaw (모델 전방이 -Z라면 +PI 보정)
    const float baseOffset = XM_PI;

    _vector curLook = XMVector3Normalize(XMVectorSetY(pTransform->GetState(MATRIXROW_LOOK), 0.f));
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
        pTransform->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, yawNow);

        if (t >= 1.f) m_yawInterpActive = false;
    }
}

void Shieldbug::Attack()
{
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

    Info* pShieldInfo = static_cast<Info*>(FindComponent(TEXT("Info")));
    _float damage = *std::get_if<_float>(pShieldInfo->GetInfo().GetPtr("AttackDamage"));

    if (ApplyDamageToPlayer(pPlayer, pPlayerInfo, damage))
    {
        if (Player* player = dynamic_cast<Player*>(pPlayer))
        {
            Transform* pShieldTf = static_cast<Transform*>(FindComponent(TEXT("Transform")));
            if (!pShieldTf)
                return;

            _vector dirKB = XMVector3Normalize(
                XMVectorSetY(pPlayerTransform->GetState(MATRIXROW_POSITION) - pShieldTf->GetState(MATRIXROW_POSITION), 0.f)
            );
            const float kbPower = 3.0f;
            const float kbTime = 0.25f;
            player->SetHit(dirKB, kbPower, kbTime);
        }
    }
}

_bool Shieldbug::ApplyDamageToPlayer(Object* pTarget, Info* info, _float damage)
{
    INFO_DESC Desc = info->GetInfo();

    _float invLeft = 0.f;
    if (auto p = Desc.GetPtr("InvincibleLeft"))
        invLeft = *std::get_if<_float>(p);
    if (invLeft > 0.f)
        return false;

    _float cur = 0.f, invT = 0.15f;
    if (auto p = Desc.GetPtr("CurHP"))
        cur = *std::get_if<_float>(p);
    if (auto p = Desc.GetPtr("InvincibleTime"))
        invT = *std::get_if<_float>(p);

    cur = max(0.f, cur - damage);
    Desc.SetData("CurHP", _float{ cur });
    Desc.SetData("IsHit", _bool{ true });
    Desc.SetData("InvincibleLeft", _float{ invT });

    if (auto pT = Desc.GetPtr("Time"))
        Desc.SetData("LastHit", *std::get_if<_float>(pT));

    info->BindInfoDesc(Desc);

    if (Transform* pTF = static_cast<Transform*>(pTarget->FindComponent(TEXT("Transform"))))
    {
        BloodHitEffect::BLOODHITEFFECT_DESC e{};
        e.fLifeTime = 0.25f;
        e.bLoop = false;
        e.bAutoKill = true;
        e.fRotationPerSec = 0.f;
        e.fSpeedPerSec = 0.f;
        XMStoreFloat3(&e.vCenterWS, pTF->GetState(MATRIXROW_POSITION));
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

_bool Shieldbug::BuildPathToTarget(const _float3& targetPos)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform || !m_pEngineUtility)
        return false;

    _vector vMyPos = pTransform->GetState(MATRIXROW_POSITION);
    _float3 myPos;
    XMStoreFloat3(&myPos, vMyPos);

    std::vector<_int> cellPath;
    if (!m_pEngineUtility->FindPath(XMLoadFloat3(&myPos), XMLoadFloat3(&targetPos), cellPath))
        return false;

    m_path.clear();

    if (!m_pEngineUtility->BuildMidWaypointsFromCellPath(
        XMLoadFloat3(&myPos), XMLoadFloat3(&targetPos), cellPath, m_path))
    {
        return false;
    }

    m_pathIndex = 0;
    return true;
}

void Shieldbug::UpdatePath(_float dt)
{
    if (!m_isMove)
        return;

    m_repathTimer -= dt;

    // 타겟(플레이어) 위치 가져오기
    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
    if (!pPlayer)
        return;

    Transform* pPlayerTf = static_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
    if (!pPlayerTf)
        return;

    _vector vPlayerPos = pPlayerTf->GetState(MATRIXROW_POSITION);
    _float3 playerPos;
    XMStoreFloat3(&playerPos, vPlayerPos);

    // 1) 아직 경로가 없거나 다 써버렸으면 → 무조건 새로 만든다
    const bool needNewPath =
        m_path.empty() ||
        m_pathIndex < 0 ||
        m_pathIndex >= (_int)m_path.size();

    // 2) 경로는 있긴 한데, 리패스 시간도 지났으면 → 새로 만든다
    if (needNewPath || m_repathTimer <= 0.f)
    {
        if (BuildPathToTarget(playerPos))
        {
            m_repathTimer = m_repathInterval;
        }
        // 실패하면 그냥 기존 경로 유지 (혹은 지워도 됨: m_path.clear(); m_pathIndex=-1;)
    }
}

void Shieldbug::HitBack(_float fTimeDelta)
{
    auto* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform)
    {
        m_kbActive = false;
        return;
    }

    const float dt = max(0.f, fTimeDelta);

    // 남은 시간 비율(1 → 0) 기반으로 감속
    const float t = (m_kbDuration > 1e-6f) ? (m_kbRemain / m_kbDuration) : 0.f;
    const float speed = m_kbPower * t;

    _vector prePos = pTransform->GetState(MATRIXROW_POSITION);
    pTransform->Translate(m_kbDir, speed * dt);

    _vector newPos = pTransform->GetState(MATRIXROW_POSITION);

    // 네비 밖으로 밀려나면 원복
    _int cellIndex = -1;
    if (!m_pEngineUtility->IsInCell(newPos, &cellIndex))
    {
        pTransform->SetState(MATRIXROW_POSITION, prePos);
    }
    else
    {
        // 네비 높이에 붙이기 (필요 없으면 생략해도 됨)
        _vector fixed{};
        m_pEngineUtility->SetHeightOnCell(newPos, &fixed);
        pTransform->SetState(MATRIXROW_POSITION, fixed);
    }

    // 문 등과 충돌 시도 원복 (Player와 동일하게 하고 싶으면)
    if (auto* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision"))))
    {
        pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

        Layer* pDoorLayer = m_pEngineUtility->FindLayer(
            m_pEngineUtility->GetCurrentSceneId(), TEXT("Door"));
        if (pDoorLayer)
        {
            for (auto& door : pDoorLayer->GetAllObjects())
            {
                if (!door) continue;
                auto* dCol = dynamic_cast<Collision*>(door->FindComponent(TEXT("Collision")));
                if (dCol && dCol->Intersect(pCollision))
                {
                    pTransform->SetState(MATRIXROW_POSITION, prePos);
                    break;
                }
            }
        }
    }

    m_kbRemain -= dt;
    if (m_kbRemain <= 0.f)
        m_kbActive = false;
}
