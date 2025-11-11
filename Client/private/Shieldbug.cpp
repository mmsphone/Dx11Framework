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

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (pModel != nullptr)
        pModel->PlayAnimation(fTimeDelta);
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
            { "Attack",     2 },
            { "Roar",       3 },
            { "Hit",        4 },
            { "Die",        5 },
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
            if (!pShieldbug || !pModel)
                return;
            pModel->SetAnimation(pShieldbug->FindAnimIndex("Roar"), false, 0.05f);
        },
        nullptr,
        nullptr
        });

    pSM->RegisterState("Idle", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pShieldbug || !pModel)
                return;
            pModel->SetAnimation(pShieldbug->FindAnimIndex("Idle"), true, 0.1f);
        },
        nullptr,
        nullptr
        });

    pSM->RegisterState("Walk", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pShieldbug || !pModel)
                return;
            pModel->SetAnimation(pShieldbug->FindAnimIndex("Walk"), true, 0.1f);
        },
        [](Object* owner, StateMachine* sm, _float fTimeDelta) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return;
            pShieldbug->Move(fTimeDelta);
        },
        nullptr
        });

    pSM->RegisterState("Attack", {
        [](Object* owner, StateMachine* sm) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pShieldbug || !pModel)
                return;
            pModel->SetAnimation(pShieldbug->FindAnimIndex("Attack"), false, 0.05f, true);
            pShieldbug->m_targetAttackable = true;
        },
        [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pShieldbug || !pModel)
                return;
            if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.6f)
            {
                pShieldbug->Attack();
                pShieldbug->m_targetAttackable = false;
            }
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
        nullptr,
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
        [](Object* owner, StateMachine* sm, _float /*dt*/) {
            auto* mdl = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!mdl)
                return;
            if (mdl->isAnimFinished())
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
            if (!pShieldbug)
                return false;
            return pShieldbug->m_isMove;
        });
    pSM->AddTransition("Idle", "Attack", FindPriorityIndex("Attack"),
        [](Object* owner, StateMachine*) {
            Shieldbug* pShieldbug = dynamic_cast<Shieldbug*>(owner);
            if (!pShieldbug)
                return false;
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
    m_pAIInputCache->SetData("PlayerPos", pTransform->GetState(POSITION));

    m_pAIInputCache->SetData("SightRange", _float{ 6.f });
    m_pAIInputCache->SetData("FovDegree", _float{ 360.f });
    m_pAIInputCache->SetData("AttackRange", _float{ 1.5f });
    m_pAIInputCache->SetData("ChasePersistTime", _float{ 3.f });
}

HRESULT Shieldbug::SetUpAIProcess()
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
            if (tf) in.SetData("DronePos", tf->GetState(POSITION));

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
                    _vector look = tf->GetState(LOOK);
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

void Shieldbug::Move(_float fTimeDelta)
{
    Transform* playerTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!playerTransform || !m_isMove) return;

    _vector prePos = playerTransform->GetState(POSITION);

    float amp = m_isSprint ? 1.4f : 1.f;
    playerTransform->Translate(m_aimDir, fTimeDelta * amp);

    _vector newPos = playerTransform->GetState(POSITION);
    if (!m_pEngineUtility->IsInCell(newPos))
        playerTransform->SetState(POSITION, prePos);

    _vector baseLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    _vector dir = XMVectorSetY(m_aimDir, 0.f);
    if (!XMVector3Equal(dir, XMVectorZero()))
    {
        float dot = std::clamp(XMVectorGetX(XMVector3Dot(baseLook, dir)), -1.0f, 1.0f);
        float ang = acosf(dot);
        _vector axis = XMVector3Cross(baseLook, dir);
        if (XMVectorGetY(axis) < 0.f)
            ang = -ang;

        playerTransform->RotateRadian(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f) + ang);
    }
}

void Shieldbug::Attack()
{
    // 히트박스 파라미터(필요시 조절)
    const float hitOffset = -0.6f;   // 드론 위치에서 전방 얼마나 떨어진 지점에 중심을 둘지
    const float hitRadius = 0.9f;   // 구 반지름
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
    _vector lookXZ = XMVector3Normalize(XMVectorSetY(pDroneTransform->GetState(LOOK), 0.f));
    _vector center = pDroneTransform->GetState(POSITION) + XMVectorScale(lookXZ, hitOffset);

    // 거리 체크(간단히 3D 거리. 필요하면 XZ 평면 거리로 바꿔도 됨)
    const _float distance = XMVectorGetX(XMVector3Length(pPlayerTransform->GetState(POSITION) - center));
    if (distance <= hitRadius)
    {
        ApplyDamageToPlayer(pPlayerInfo, damage);
        if (Player* player = dynamic_cast<Player*>(pPlayer))
        {
            _vector dirKB = XMVector3Normalize(XMVectorSetY(pPlayerTransform->GetState(POSITION) - pDroneTransform->GetState(POSITION), 0.f));
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