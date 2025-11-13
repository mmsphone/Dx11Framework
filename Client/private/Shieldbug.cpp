#include "Shieldbug.h"

#include "EngineUtility.h"

#include "Player.h"

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

    pDesc->fSpeedPerSec = 2.f;
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

    return S_OK;
}

void Shieldbug::Update(_float fTimeDelta)
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
}

void Shieldbug::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::SHADOWLIGHT, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Shieldbug::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix"))) return E_FAIL;
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
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            auto* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (pModel) pModel->PlayAnimation(fTimeDelta);

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
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->SetAnimation(pShieldbug->FindAnimIndex("Attack"), false, 0.05f, true);
            pShieldbug->m_targetAttackable = true;
        },
        [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.6f)
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
        },
         [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));

            pModel->PlayAnimation(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Die", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pShieldbug || !pModel)
                return;
            pModel->SetAnimation(pShieldbug->FindAnimIndex("Die"), false, 0.05f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            auto* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            
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
            if (pModel->isAnimFinished() && *std::get_if<_bool>(pInfo->GetInfo().GetPtr("IsHit")))
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
    desc.SetData("MaxHP", _float{ 120.f });
    desc.SetData("CurHP", _float{ 120.f });
    desc.SetData("InvincibleTime", _float{ 0.15f });
    desc.SetData("InvincibleLeft", _float{ 0.f });
    desc.SetData("Time", _float{ 0.f });
    desc.SetData("LastHit", _float{ -999.f });
    desc.SetData("IsHit", _bool{ false });
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
    m_pAIInputCache->SetData("TrackRange", _float{ 4.5f });
    m_pAIInputCache->SetData("FovDegree", _float{ 360.f });
    m_pAIInputCache->SetData("AttackRange", _float{ 2.5f });
    m_pAIInputCache->SetData("ChasePersistTime", _float{ 3.f });
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

                const float sightRange = in.GetPtr("SightRange") ? std::get<_float>(*in.GetPtr("SightRange")) : 8.f;
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

            const float trackRange = in.GetPtr("TrackRange") ? std::get<_float>(*in.GetPtr("TrackRange")) : 4.5f;
            const float attackRange = in.GetPtr("AttackRange") ? std::get<_float>(*in.GetPtr("AttackRange")) : 2.5f;

            const bool inSight = visible;
            const bool inTrack = (distance <= trackRange);
            const bool inAttack = (distance <= attackRange);

            if (inSight && !inTrack)
            {
                out.SetData("isDefend", _bool{ true });
            }
            else if ((inSight || chaseAlive) && inTrack && !inAttack)
            {
                out.SetData("isMove", _bool{ true });
                out.SetData("moveDir", dir);
            }
            else if ((inSight || chaseAlive) && inAttack)
            {
                out.SetData("isAttack", _bool{ true });
            }
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
    Transform* playerTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!playerTransform || !m_isMove) return;

    _vector prePos = playerTransform->GetState(MATRIXROW_POSITION);

    playerTransform->Translate(m_aimDir, fTimeDelta);

    _vector newPos = playerTransform->GetState(MATRIXROW_POSITION);
    if (!m_pEngineUtility->IsInCell(newPos))
        playerTransform->SetState(MATRIXROW_POSITION, prePos);
    
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
    // 히트박스 파라미터(필요시 조절)
    const float hitOffset = -1.7f;   // 드론 위치에서 전방 얼마나 떨어진 지점에 중심을 둘지
    const float hitRadius = 0.8f;   // 구 반지름
    const float damage = 15.f;   // 대미지량

    Transform* pDroneTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pDroneTransform)
        return;

    // 플레이어 찾기
    const _uint sceneId = m_pEngineUtility->GetCurrentSceneId();
    Object* pPlayer = m_pEngineUtility->FindObject(sceneId, TEXT("Player"), 0);
    if (!pPlayer)
        return;

    Transform* pPlayerTransform = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
    Info* pPlayerInfo = dynamic_cast<Info*>(pPlayer->FindComponent(TEXT("Info")));
    if (!pPlayerTransform || !pPlayerInfo)
        return;

    // 전방 오프셋 + 구 히트박스
    _vector lookXZ = XMVector3Normalize(XMVectorSetY(pDroneTransform->GetState(MATRIXROW_LOOK), 0.f));
    _vector center = pDroneTransform->GetState(MATRIXROW_POSITION) + XMVectorScale(lookXZ, hitOffset);

    // 거리 체크(간단히 3D 거리. 필요하면 XZ 평면 거리로 바꿔도 됨)
    const _float distance = XMVectorGetX(XMVector3Length(pPlayerTransform->GetState(MATRIXROW_POSITION) - center));
    if (distance <= hitRadius)
    {
        ApplyDamageToPlayer(pPlayerInfo, damage);
        if (Player* player = dynamic_cast<Player*>(pPlayer))
        {
            _vector dirKB = XMVector3Normalize(XMVectorSetY(pPlayerTransform->GetState(MATRIXROW_POSITION) - pDroneTransform->GetState(MATRIXROW_POSITION), 0.f));
            const float kbPower = 3.0f;
            const float kbTime = 0.2f;
            player->SetHit(dirKB, kbPower, kbTime);
        }
    }
}

void Shieldbug::ApplyDamageToPlayer(Info* info, _float damage)
{
    INFO_DESC Desc = INFO_DESC{};
    Desc = info->GetInfo();

    _float invLeft = 0.f;
    if (auto p = Desc.GetPtr("InvincibleLeft")) invLeft = *std::get_if<_float>(p);
    if (invLeft > 0.f)
        return;

    _float cur = 0.f, invT = 0.15f;
    if (auto p = Desc.GetPtr("CurHP"))            cur = *std::get_if<_float>(p);
    if (auto p = Desc.GetPtr("InvincibleTime"))   invT = *std::get_if<_float>(p);

    cur = max(0.f, cur - damage);
    Desc.SetData("CurHP", _float{ cur });
    Desc.SetData("IsHit", _bool{ true });
    Desc.SetData("InvincibleLeft", _float{ invT });

    if (auto pT = Desc.GetPtr("Time"))
        Desc.SetData("LastHit", *std::get_if<_float>(pT));

}