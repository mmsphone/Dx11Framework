#include "Worm.h"

#include "EngineUtility.h"

#include "Worm_Projectile.h"
#include "Layer.h"
#include "BloodHitEffect.h"
#include "BloodDieEffect.h"
#include "Player.h"

Worm::Worm() 
    : ObjectTemplate{} 
{
}

Worm::Worm(const Worm& Prototype) 
    : ObjectTemplate{ Prototype } 
{
}

HRESULT Worm::Initialize(void* pArg)
{
    OBJECT_DESC* pDesc = new OBJECT_DESC;

    pDesc->fSpeedPerSec = 0.f;
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

    m_isDying = false;
    m_deathFade = 1.f;

    return S_OK;
}

void Worm::Update(_float fTimeDelta)
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

    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
}

void Worm::LateUpdate(_float fTimeDelta)
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

HRESULT Worm::Render()
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
                pShader->Begin(0);
            else
                pShader->Begin(5);
            pModel->Render(i);
        }
    }

#ifdef _DEBUG
    Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Render();
#endif

    return S_OK;
}

HRESULT Worm::RenderShadow(_uint iIndex)
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
_uint Worm::FindAnimIndex(string animName, _uint fallback) const
{
    auto it = m_animIndexMap.find(animName);
    if (it != m_animIndexMap.end())
        return it->second;
    return fallback;
}

_uint Worm::FindPriorityIndex(string toStateName, _uint fallback) const
{
    auto it = m_priorityIndexMap.find(toStateName);
    if (it != m_priorityIndexMap.end())
        return it->second;
    return fallback;
}

void Worm::SetHit(_vector dirXZ, float power, float duration)
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

Worm* Worm::Create()
{
    Worm* pInstance = new Worm();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Worm");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Object* Worm::Clone(void* pArg)
{
    Worm* pInstance = new Worm(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Worm");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Worm::Free()
{
    __super::Free();

    SafeDelete(m_pAIInputCache);
}

HRESULT Worm::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Worm"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL; 
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("StateMachine"), TEXT("StateMachine"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("AIController"), TEXT("AIController"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Info"), TEXT("Info"), nullptr, nullptr)))
        return E_FAIL;

    CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
    OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
    XMStoreFloat3(&OBBDesc.vExtents, _vector{ 0.2f, 1.f, 0.2f } / scaleOffset);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
        return E_FAIL;

    return S_OK;
}

void Worm::SetUpAnimIndexMap()
{
    m_animIndexMap = {
        { "Attack",     0 },
        { "Die",        1 },
        { "Roar",        2 },
        { "Hit",       3 },
        { "Idle",       4 },
    };
}

void Worm::SetUpPriorityIndexMap()
{
    m_priorityIndexMap = {
            { "Idle",       0 },
            { "Attack",     1 },
            { "Roar",       2 },
            { "Hit",        3 },
            { "Die",        4 },
    };
}

HRESULT Worm::SetUpStateMachine()
{
    StateMachine* pSM = dynamic_cast<StateMachine*>(FindComponent(TEXT("StateMachine")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pSM || !pModel)
        return E_FAIL;
    
    //상태 등록
    pSM->RegisterState("Roar", {
        [](Object* owner, StateMachine* sm) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pWorm->FindAnimIndex("Roar"), false, 0.05f);

            _float r = EngineUtility::GetInstance()->Random(0, 2);
            if (r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneRoar1");
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneRoar2");
        },
         [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pWorm->Rotate(fTimeDelta);
            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Idle", {
       [](Object* owner, StateMachine* sm) {
           Worm* pWorm = dynamic_cast<Worm*>(owner);
           Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

           pModel->SetAnimation(pWorm->FindAnimIndex("Idle"), true, 0.1f);
       },
       [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pWorm->Rotate(fTimeDelta);
            pModel->PlayAnimation(fTimeDelta);
        },
       nullptr
    });

    pSM->RegisterState("Attack", {
            [](Object* owner, StateMachine* sm) {
                Worm* pWorm = dynamic_cast<Worm*>(owner);
                Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

                pModel->SetAnimation(pWorm->FindAnimIndex("Attack"), false, 0.05f, true);
                pWorm->m_targetAttackable = true;
            },
            [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
                Worm* pWorm = dynamic_cast<Worm*>(owner);
                Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

                pModel->PlayAnimation(fTimeDelta);
                if (pWorm->m_targetAttackable && pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.4f)
                {
                    pWorm->Shoot();
                    pWorm->m_targetAttackable = false;
                }
                else if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.2f)
                {
                    pWorm->Rotate(fTimeDelta);
                }
            },
            nullptr
    });

    pSM->RegisterState("Hit", {
        [](Object* owner, StateMachine* sm) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pWorm->FindAnimIndex("Hit"), false, 0.05f, true);

            _float r = EngineUtility::GetInstance()->Random(0, 12);
            if (r >= 11)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit1");
            else if (r >= 10)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit2");
            else if (r >= 9)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit3");
            else if (r >= 8)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit4");
            else if (r >= 7)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit5");
            else if (r >= 6)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit6");
            else if (r >= 5)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit7");
            else if (r >= 4)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit8");
            else if (r >= 3)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit9");
            else if (r >= 2)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit10");
            else if (r >= 1)
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit11");
            else
                EngineUtility::GetInstance()->PlaySound2D("FBX_droneHit12");
        },
        [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
    });

    pSM->RegisterState("Die", {
        [](Object* owner, StateMachine* sm) {
            Worm* pWorm = static_cast<Worm*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pWorm->FindAnimIndex("Die"), false, 0.05f);

            Info* pInfo = static_cast<Info*>(owner->FindComponent(TEXT("Info")));
            INFO_DESC desc = pInfo->GetInfo();
            desc.SetData("InvincibleLeft", _float{ 10.f });
            pInfo->BindInfoDesc(desc);

            pWorm->m_isDying = true;
            pWorm->m_deathFade = 1.f;

            Collision* pCol = static_cast<Collision*>(owner->FindComponent(TEXT("Collision")));
            _float3 centerPos = static_cast<BoundingOrientedBox*>( pCol->GetWorldCollisionBox(COLLISIONTYPE_OBB))->Center;

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
                ev.tag = L"Worm";
                EngineUtility::GetInstance()->PushEvent(ev);
            }

            EngineUtility::GetInstance()->PlaySound2D("FBX_wormDie1");
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Worm* pMonster = dynamic_cast<Worm*>(owner);
            Model* pModel = static_cast<Model*>(owner->FindComponent(TEXT("Model")));

            const _float dieAnimFaster = 10.f;
            pModel->PlayAnimation(fTimeDelta * dieAnimFaster);

            float pos = pModel->GetCurAnimTrackPos();
            float dur = pModel->GetCurAnimDuration();
            float t = 0.f;
            if (dur > 1e-4f)
                t = clamp(pos / dur, 0.f, 1.f);
            float eased = t * t;
            pMonster->m_deathFade = 1.f - eased;

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
    pSM->AddTransition("Idle", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            return pWorm->m_isAttack;
        });
    pSM->AddTransition("Idle", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pWorm->FindComponent(TEXT("Info")));
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
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pWorm->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Attack ->
    pSM->AddTransition("Attack", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pWorm->m_isAttack;
        });
    pSM->AddTransition("Attack", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && pWorm->m_isAttack;
        }, true);
    pSM->AddTransition("Attack", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pWorm->FindComponent(TEXT("Info")));
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
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pWorm->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    //Hit ->
    pSM->AddTransition("Hit", "Idle", FindPriorityIndex("Idle"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && !pWorm->m_isAttack;
        });
    pSM->AddTransition("Hit", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            return pModel->isAnimFinished() && pWorm->m_isAttack;
        }, true);
    pSM->AddTransition("Hit", "Hit", FindPriorityIndex("Hit"),
        [](Object* owner, StateMachine*) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pModel)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pWorm->FindComponent(TEXT("Info")));
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
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            if (!pWorm)
                return false;
            Info* pInfo = dynamic_cast<Info*>(pWorm->FindComponent(TEXT("Info")));
            if (!pInfo)
                return false;
            return pInfo->IsDead();
        });

    pSM->SetState("Roar");

    return S_OK;
}

HRESULT Worm::SetUpInfo()
{
    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (!pInfo)
        return E_FAIL;

    INFO_DESC desc;
    desc.SetData("MaxHP", _float{ 30.f });
    desc.SetData("CurHP", _float{ 30.f });
    desc.SetData("InvincibleTime", _float{ 0.1f });
    desc.SetData("InvincibleLeft", _float{ 0.f });
    desc.SetData("Time", _float{ 0.f });
    desc.SetData("LastHit", _float{ -999.f });
    desc.SetData("IsHit", _bool{ false });
    desc.SetData("Faction", FACTION_MONSTER);
    desc.SetData("AttackDamage", _float{ 10.f });
    pInfo->BindInfoDesc(desc);

    return S_OK;
}

void Worm::SetUpAIInputData()
{
    Object* pPlayerPos = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), TEXT("Player"), 0);
    if (!pPlayerPos)
        return;
    Transform* pTransform = dynamic_cast<Transform*>(pPlayerPos->FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;
    m_pAIInputCache->SetData("PlayerPos", pTransform->GetState(MATRIXROW_POSITION));

    m_pAIInputCache->SetData("SightRange", _float{ 10.f });
    m_pAIInputCache->SetData("FovDegree", _float{ 360.f });
    m_pAIInputCache->SetData("AttackRange", _float{ 8.f });
    m_pAIInputCache->SetData("ChasePersistTime", _float{ 0.f });
}

HRESULT Worm::SetUpAIProcess()
{
    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (!pAI) return E_FAIL;

    AIPROCESS_DESC proc{};

    // [1] SENSE
    proc.sense = [this](AIINPUT_DESC& in, _float /*dt*/, _float time)
        {
            Transform* tf = dynamic_cast<Transform*>(this->FindComponent(TEXT("Transform")));
            if (tf) in.SetData("WormPos", tf->GetState(MATRIXROW_POSITION));

            const AIValue* pWormPos = in.GetPtr("WormPos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");

            in.SetData("Visible", _bool{ false });

            if (pWormPos && pPlayerPos)
            {
                const _vector wormPos = std::get<_vector>(*pWormPos);
                const _vector playerPos = std::get<_vector>(*pPlayerPos);

                const _float distance = XMVectorGetX(XMVector3Length(playerPos - wormPos));
                in.SetData("Distance", _float{ distance });

                float sightRange = in.GetPtr("SightRange") ? std::get<_float>(*in.GetPtr("SightRange")) : 10.f;
                _float globalSightScale = static_cast<Player*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0))->GetGlobalSightScale();
                sightRange *= globalSightScale;
                const float fovDeg = in.GetPtr("FovDegree") ? std::get<_float>(*in.GetPtr("FovDegree")) : 360.f;

                bool inRangeSight = (distance <= sightRange);

                bool inFov = true;
                if (fovDeg < 359.f && tf)
                {
                    _vector look = tf->GetState(MATRIXROW_LOOK);
                    _vector toT = XMVector3Normalize(playerPos - wormPos);
                    look = XMVector3Normalize(XMVectorSet(XMVectorGetX(look), 0.f, XMVectorGetZ(look), 0.f));
                    toT = XMVector3Normalize(XMVectorSet(XMVectorGetX(toT), 0.f, XMVectorGetZ(toT), 0.f));
                    const float cosang = std::clamp(XMVectorGetX(XMVector3Dot(look, toT)), -1.f, 1.f);
                    const float angDeg = XMConvertToDegrees(acosf(cosang));
                    inFov = (angDeg <= (fovDeg * 0.5f));
                }

                const bool visible = (inRangeSight && inFov);
                in.SetData("Visible", _bool{ visible });
                if (visible) in.SetData("LastSeenSec", _float{ time });
            }
        };

    // [2] DECIDE
    proc.decide = [this](const AIINPUT_DESC& in, AIOUTPUT_DESC& out, _float /*dt*/, _float time)
        {
            out.SetData("isAttack", _bool{ false });

            const AIValue* pWormPos = in.GetPtr("WormPos"); // ★ fix
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");
            if (!pWormPos || !pPlayerPos) return;

            const bool  visible = in.GetPtr("Visible") ? std::get<_bool>(*in.GetPtr("Visible")) : false;
            const float distance = in.GetPtr("Distance") ? std::get<_float>(*in.GetPtr("Distance")) : 1e9f;
            const float atkRange = in.GetPtr("AttackRange") ? std::get<_float>(*in.GetPtr("AttackRange")) : 4.f;

            // Info에서 쿨다운 읽기
            float nextT = -1e9f, cooldown = 1.6f;
            if (Info* info = dynamic_cast<Info*>(this->FindComponent(TEXT("Info"))))
            {
                auto desc = info->GetInfo();
                if (auto pN = desc.GetPtr("NextAttackT"))    nextT = *std::get_if<_float>(pN);
                if (auto pC = desc.GetPtr("AttackCooldown")) cooldown = *std::get_if<_float>(pC);
            }

            const bool inRange = (distance <= atkRange);
            const bool canShoot = (time >= nextT);

            // 항상 aimDir 유지(가시 중엔 바라보기)
            {
                const _vector wormPos = std::get<_vector>(*pWormPos);
                const _vector playerPos = std::get<_vector>(*pPlayerPos);
                _vector aim = XMVector3Normalize(playerPos - wormPos);
                aim = XMVector3Normalize(XMVectorSet(XMVectorGetX(aim), 0.f, XMVectorGetZ(aim), 0.f));
                out.SetData("aimDir", aim);
            }

            if (visible && inRange && canShoot)
            {
                out.SetData("isAttack", _bool{ true });
                out.SetData("attackCooldown", _float{ cooldown }); // (선택) Attack에서 참고 가능
            }
        };

    // [3] ACT
    proc.act = [this](AIOUTPUT_DESC& out, _float /*dt*/, _float /*time*/)
        {
            if (auto p = out.GetPtr("aimDir")) {
                _vector v = std::get<_vector>(*p);
                const float len = XMVectorGetX(XMVector3Length(v));
                if (len > 1e-6f) out.SetData("aimDir", XMVectorScale(v, 1.f / len));
            }
        };

    // [4] APPLY
    proc.applyOutput = [this](const AIOUTPUT_DESC& out)
        {
            m_isAttack = false;
            if (auto p = out.GetPtr("isAttack")) m_isAttack = std::get<_bool>(*p);
            if (auto p = out.GetPtr("aimDir"))   m_aimDir = std::get<_vector>(*p);
        };

    pAI->BindProcessDesc(proc);
    return S_OK;
}

void Worm::Rotate(_float fTimeDelta)
{
    Transform* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!tf) return;

    // 1) 원하는 목표 방향(수평)
    _vector desiredDir = m_aimDir;

    if (XMVector3Equal(desiredDir, XMVectorZero()))
    {
        // m_aimDir이 비어있으면 플레이어 위치 기반 계산
        const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
        if (Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0))
        {
            if (Transform* pPT = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform"))))
            {
                _vector wormPos = tf->GetState(MATRIXROW_POSITION);
                _vector playerPos = pPT->GetState(MATRIXROW_POSITION);
                desiredDir = XMVector3Normalize(XMVectorSetY(playerPos - wormPos, 0.f));
            }
        }
    }

    if (XMVector3Equal(desiredDir, XMVectorZero()))
        return;

    desiredDir = XMVector3Normalize(desiredDir);

    // 2) 현재/목표 yaw 계산 (모델 전방 -Z 보정: +PI)
    const float baseOffset = XM_PI;

    _vector curLook = XMVector3Normalize(XMVectorSetY(tf->GetState(MATRIXROW_LOOK), 0.f));
    float curYaw = atan2f(XMVectorGetX(curLook), XMVectorGetZ(curLook));     // [-PI,PI]
    float dstYaw = baseOffset + atan2f(XMVectorGetX(desiredDir), XMVectorGetZ(desiredDir));
    while (dstYaw > XM_PI) dstYaw -= XM_2PI;
    while (dstYaw < -XM_PI) dstYaw += XM_2PI;

    float shortestDelta = dstYaw - m_yawTarget;
    while (shortestDelta > XM_PI) shortestDelta -= XM_2PI;
    while (shortestDelta < -XM_PI) shortestDelta += XM_2PI;

    // 3) 목표가 바뀌었으면 새 보간 시작
    const float deltaNow = fabsf(shortestDelta);
    const float reTargetEps = XMConvertToRadians(2.0f); // 2도 이상 차이나면 재타깃팅
    if (!m_yawInterpActive || deltaNow > reTargetEps)
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
        float s = t * t * (3.f - 2.f * t);

        // 최단각 기준으로 보간
        float shortestDelta = m_yawTarget - m_yawStart;
        while (shortestDelta > XM_PI) shortestDelta -= XM_2PI;
        while (shortestDelta < -XM_PI) shortestDelta += XM_2PI;
        float d = shortestDelta;

        float yawNow = m_yawStart + d * s;
        while (yawNow > XM_PI) yawNow -= XM_2PI;
        while (yawNow < -XM_PI) yawNow += XM_2PI;

        // 절대 yaw 설정(너의 Transform::RotateRadian이 절대각 세팅 방식이라는 전제—플레이어 코드와 동일 패턴)
        tf->RotateRadian(_vector{ 0.f,1.f,0.f,0.f }, yawNow);

        if (t >= 1.f) {
            m_yawInterpActive = false;
        }
    }
}

void Worm::Shoot()
{
    Transform* pWormTF = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pWormTF) return;

    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
    if (!pPlayer) return;

    Transform* pPlayerTF = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
    if (!pPlayerTF) return;

    const _vector wormPos = pWormTF->GetState(MATRIXROW_POSITION);
    const _vector playerPos = pPlayerTF->GetState(MATRIXROW_POSITION);

    _vector moveDir = XMVector3Normalize(XMVectorSetY(playerPos - wormPos, 0.f));
    if (XMVector3Equal(moveDir, XMVectorZero()))
        moveDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);

    _vector spawnPos = wormPos;
    bool setBySocket = false;

    // 모델 소켓 시도 (예: "mouth" 또는 "muzzle" 이름 가정)
    if (Model* mdl = dynamic_cast<Model*>(FindComponent(TEXT("Model"))))
    {
        const _float4x4* pNodeMat = mdl->GetSocketBoneMatrixPtr("Head");
        if (pNodeMat)
        {
            _matrix mNode = XMLoadFloat4x4(pNodeMat);
            _matrix mWorld = XMLoadFloat4x4(pWormTF->GetWorldMatrixPtr());
            spawnPos = XMVector3TransformCoord(mNode.r[3], mWorld);
            setBySocket = true;
        }
    }

    if (!setBySocket)
    {
        return;
    }

    _vector aimPos = XMVectorSetY(playerPos, XMVectorGetY(playerPos) + 0.5f);

    Projectile::PROJECTILE_DESC Desc{};
    Desc.moveDir = aimPos - spawnPos;
    Desc.accTime = 0.f;
    Info* pInfo = static_cast<Info*>(FindComponent(TEXT("Info")));
    _float damage = *std::get_if<_float>(pInfo->GetInfo().GetPtr("AttackDamage"));
    Desc.damageAmount = damage;
    Desc.faction = FACTION_MONSTER;
    Desc.hitRadius = 0.6f;
    Desc.lifeTime = 2.0f;
    Desc.fSpeedPerSec = 20.0f;

    if (FAILED(m_pEngineUtility->AddObject(sceneId, TEXT("Worm_Projectile"), sceneId, TEXT("Projectile"), &Desc)))
        return;

    {
        auto* pProjLayer = m_pEngineUtility->FindLayer(sceneId, TEXT("Projectile"));
        if (pProjLayer && !pProjLayer->GetAllObjects().empty())
        {
            Object* pNewProj = pProjLayer->GetAllObjects().back();
            if (Transform* pProjTF = dynamic_cast<Transform*>(pNewProj->FindComponent(TEXT("Transform"))))
                pProjTF->SetState(MATRIXROW_POSITION, spawnPos);
        }
    }
}

void Worm::HitBack(_float fTimeDelta)
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
