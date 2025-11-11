#include "Worm.h"
#include "EngineUtility.h"

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

    return S_OK;
}

void Worm::Update(_float fTimeDelta)
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

void Worm::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::SHADOWLIGHT, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Worm::Render()
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

HRESULT Worm::RenderShadow(_uint iIndex)
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
            if (!pWorm || !pModel)
                return;
            pModel->SetAnimation(pWorm->FindAnimIndex("Roar"), false, 0.05f);
        },
        nullptr,
        nullptr
    });

    pSM->RegisterState("Idle", {
       [](Object* owner, StateMachine* sm) {
           Worm* pWorm = dynamic_cast<Worm*>(owner);
           Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
           if (!pWorm || !pModel)
               return;
           pModel->SetAnimation(pWorm->FindAnimIndex("Idle"), true, 0.1f);
       },
       [](Object* owner, StateMachine* /*sm*/, _float /*dt*/) {
            auto* worm = dynamic_cast<Worm*>(owner);
            if (!worm) return;

            Transform* tf = dynamic_cast<Transform*>(owner->FindComponent(TEXT("Transform")));
            if (!tf) return;

            // m_moveDir을 조준 벡터로 재활용
            _vector dir = XMVectorSetY(worm->m_aimDir, 0.f);
            if (!XMVector3Equal(dir, XMVectorZero()))
            {
                _vector baseLook = XMVectorSet(0.f, 0.f, 1.f, 0.f);
                float dot = std::clamp(XMVectorGetX(XMVector3Dot(baseLook, XMVector3Normalize(dir))), -1.0f, 1.0f);
                float ang = acosf(dot);
                _vector axis = XMVector3Cross(baseLook, dir);
                if (XMVectorGetY(axis) < 0.f) ang = -ang;

                // 전방축이 -Z면 +180 유지(드론과 동일한 축 기준 가정)
                tf->RotateRadian(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f) + ang);
            }
        },
       nullptr
    });

    pSM->RegisterState("Attack", {
            [](Object* owner, StateMachine* sm) {
                Worm* pWorm = dynamic_cast<Worm*>(owner);
                Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
                if (!pWorm || !pModel)
                    return;
                pModel->SetAnimation(pWorm->FindAnimIndex("Attack"), false, 0.05f, true);
                pWorm->m_targetAttackable = true;
            },
            [](Object* owner, StateMachine* /*sm*/, _float fTimeDelta) {
                Worm* pWorm = dynamic_cast<Worm*>(owner);
                Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
                if (!pWorm || !pModel)
                    return;
                if (pModel->GetCurAnimTrackPos() > pModel->GetCurAnimDuration() * 0.3f)
                {
                    pWorm->Shoot();
                    pWorm->m_targetAttackable = false;
                }
            },
            nullptr
    });

    pSM->RegisterState("Hit", {
        [](Object* owner, StateMachine* sm) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pWorm || !pModel)
                return;
            pModel->SetAnimation(pWorm->FindAnimIndex("Hit"), false, 0.05f, true);
        },
        nullptr,
        nullptr
    });

    pSM->RegisterState("Die", {
        [](Object* owner, StateMachine* sm) {
            Worm* pWorm = dynamic_cast<Worm*>(owner);
            Model* pModel = dynamic_cast<Model*>(owner->FindComponent(TEXT("Model")));
            if (!pWorm || !pModel)
                return;
            pModel->SetAnimation(pWorm->FindAnimIndex("Die"), false, 0.05f);
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

void Worm::SetUpAIInputData()
{
    Object* pPlayerPos = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), TEXT("Player"), 0);
    if (!pPlayerPos)
        return;
    Transform* pTransform = dynamic_cast<Transform*>(pPlayerPos->FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;
    m_pAIInputCache->SetData("PlayerPos", pTransform->GetState(POSITION));

    m_pAIInputCache->SetData("SightRange", _float{ 8.f });
    m_pAIInputCache->SetData("FovDegree", _float{ 360.f });
    m_pAIInputCache->SetData("AttackRange", _float{ 4.f });
    m_pAIInputCache->SetData("ChasePersistTime", _float{ 0.f });
}

HRESULT Worm::SetUpAIProcess()
{
    AIController* pAI = dynamic_cast<AIController*>(FindComponent(TEXT("AIController")));
    if (!pAI)
        return E_FAIL;

    AIPROCESS_DESC proc{};

    // [1] SENSE: 입력 보강만 담당 (this에서 Transform 조회)
    proc.sense = [this](AIINPUT_DESC& in, _float /*dt*/, _float time)
        {
            Transform* tf = dynamic_cast<Transform*>(this->FindComponent(TEXT("Transform")));

            // WormPos
            if (tf) in.SetData("WormPos", tf->GetState(POSITION));

            const AIValue* pWormPos = in.GetPtr("WormPos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");

            // 기본 가시성 false
            in.SetData("Visible", _bool{ false });

            if (pWormPos && pPlayerPos)
            {
                const _vector wormPos = std::get<_vector>(*pWormPos);
                const _vector playerPos = std::get<_vector>(*pPlayerPos);

                // 거리
                const _float distance = XMVectorGetX(XMVector3Length(playerPos - wormPos));
                in.SetData("Distance", _float{ distance });

                // 파라미터(없으면 기본값)
                const float sightRange = in.GetPtr("SightRange") ? std::get<_float>(*in.GetPtr("SightRange")) : 8.f;
                const float fovDeg = in.GetPtr("FovDegree") ? std::get<_float>(*in.GetPtr("FovDegree")) : 360.f;

                // 가시성 (XZ 평면 FOV)
                bool inRangeSight = (distance <= sightRange);

                bool inFov = true;
                if (fovDeg < 359.f && tf)
                {
                    _vector look = tf->GetState(LOOK);
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

    // [2] DECIDE: 파생 판단(out: inRange/tracking/isMove/attack)
    proc.decide = [this](const AIINPUT_DESC& in, AIOUTPUT_DESC& out, _float /*dt*/, _float time)
        {
            out.SetData("isAttack", _bool{ false });

            const AIValue* pWormPos = in.GetPtr("DronePos");
            const AIValue* pPlayerPos = in.GetPtr("PlayerPos");
            if (!pWormPos || !pPlayerPos) 
                return;

            const bool  visible = in.GetPtr("Visible") ? std::get<_bool>(*in.GetPtr("Visible")) : false;
            const float distance = in.GetPtr("Distance") ? std::get<_float>(*in.GetPtr("Distance")) : 1e9f;
            const float atkRange = in.GetPtr("AttackRange") ? std::get<_float>(*in.GetPtr("AttackRange")) : 9.f;
            const float cooldown = in.GetPtr("AttackCooldown") ? std::get<_float>(*in.GetPtr("AttackCooldown")) : 1.6f;

            float nextT = -1e9f;
            if (Info* info = dynamic_cast<Info*>(this->FindComponent(TEXT("Info"))))
            {
                const InfoValue* pN = info->GetInfo().GetPtr("NextAttackT");
                if (pN) nextT = std::get<_float>(*pN);
            }

            const bool inRange = (distance <= atkRange);
            const bool canShoot = (time >= nextT);

            if (visible && inRange && canShoot)
            {
                // 조준 방향(수평)
                const _vector wormPos = std::get<_vector>(*pWormPos);
                const _vector playerPos = std::get<_vector>(*pPlayerPos);
                _vector aim = XMVector3Normalize(playerPos - wormPos);
                aim = XMVector3Normalize(XMVectorSet(XMVectorGetX(aim), 0.f, XMVectorGetZ(aim), 0.f));

                out.SetData("isAttack", _bool{ true });
                out.SetData("aimDir", aim); // 상태머신에서 회전/발사 타이밍에 사용
                out.SetData("attackCooldown", _float{ cooldown });
            }
            else
            {
                // 가시 중이면 바라보기만 하도록 aimDir은 계속 내보내 줌
                if (visible)
                {
                    const _vector wormPos = std::get<_vector>(*pWormPos);
                    const _vector playerPos = std::get<_vector>(*pPlayerPos);
                    _vector aim = XMVector3Normalize(playerPos - wormPos);
                    aim = XMVector3Normalize(XMVectorSet(XMVectorGetX(aim), 0.f, XMVectorGetZ(aim), 0.f));
                    out.SetData("aimDir", aim);
                }
            }
        };

    // [3] ACT: 출력 보정
    proc.act = [this](AIOUTPUT_DESC& out, _float /*dt*/, _float /*time*/)
        {
            if (auto p = out.GetPtr("aimDir")) {
                _vector v = std::get<_vector>(*p);
                const float len = XMVectorGetX(XMVector3Length(v));
                if (len > 1e-6f) out.SetData("aimDir", XMVectorScale(v, 1.f / len));
            }
        };

    // [4] APPLY: Drone 멤버 반영
    proc.applyOutput = [this](const AIOUTPUT_DESC& out)
        {
            m_isAttack = false;
            if (auto p = out.GetPtr("isAttack")) m_isAttack = std::get<_bool>(*p);
            if (auto p = out.GetPtr("aimDir"))   m_aimDir = std::get<_vector>(*p);
        };

    pAI->BindProcessDesc(proc);
    return S_OK;
}

void Worm::Shoot()
{

}