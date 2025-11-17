#include "Worm_Projectile.h"

#include "EngineUtility.h"

#include "Player.h"
#include "BloodHitEffect.h"

Worm_Projectile::Worm_Projectile()
    : Projectile{}
{
}

Worm_Projectile::Worm_Projectile(const Worm_Projectile& Prototype)
    : Projectile{ Prototype }
{
}

HRESULT Worm_Projectile::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void Worm_Projectile::Update(_float fTimeDelta)
{
    // 수명, 이동 처리 상위 클래스에서 함
    __super::Update(fTimeDelta);
    if (IsDead())
        return;

    // 충돌 체크
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    if (m_desc.hitRadius <= 0.f)
        return;

    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

    std::vector<Object*> hits;
    HitProjectile(hits);

    for (Object* obj : hits)
    {
        if (obj == nullptr || obj == this)
            continue;
        if (TryApplyHitTo(obj))
        {
            SetDead(true);
            break;
        }
    }
}

void Worm_Projectile::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDER_BLEND, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Worm_Projectile::Render()
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));
    VIBufferInstancingPoint* pVIBuffer = static_cast<VIBufferInstancingPoint*>(FindComponent(TEXT("VIBuffer")));

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;

    _float4 bulletColor = { 0.2f, 1.0f, 0.2f, 1.0f }; // 초록
    if (FAILED(pShader->BindRawValue("g_vProjectileColor", &bulletColor, sizeof(_float4))))
        return E_FAIL;

    _vector vDir = XMVector3Normalize(m_desc.moveDir);
    _float3 dir3{};
    XMStoreFloat3(&dir3, vDir);
    if (FAILED(pShader->BindRawValue("g_vProjectileDir", &dir3, sizeof(_float3))))
        return E_FAIL;

    _float trailLen = min(m_desc.accTime * m_desc.fSpeedPerSec, 1.5f);
    if (FAILED(pShader->BindRawValue("g_fTrailLength", &trailLen, sizeof(_float))))
        return E_FAIL;

    // 총알 사각형 크기
    _float projectileSize = 0.05f;
    if (FAILED(pShader->BindRawValue("g_fProjectileSize", &projectileSize, sizeof(_float))))
        return E_FAIL;

    pShader->Begin(0);

    pVIBuffer->BindBuffers();
    pVIBuffer->Render();

#ifdef _DEBUG
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Render();
#endif

    return S_OK;
}

Worm_Projectile* Worm_Projectile::Create()
{
    Worm_Projectile* pInstance = new Worm_Projectile();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Worm_Projectile");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* Worm_Projectile::Clone(void* pArg)
{
    Worm_Projectile* pInstance = new Worm_Projectile(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Worm_Projectile");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void Worm_Projectile::Free()
{
    __super::Free();
}

HRESULT Worm_Projectile::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_Particle_Bullet"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxProjectile"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    CollisionBoxSphere::COLLISIONSPHERE_DESC     SphereDesc{};
    SphereDesc.vCenter = _float3(0.f, 0.f, 0.f);
    SphereDesc.fRadius = 0.15f;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionSphere"), TEXT("Collision"), nullptr, &SphereDesc)))
        return E_FAIL;

    return S_OK;
}

_bool Worm_Projectile::TryApplyHitTo(Object* pTarget)
{
    // 대상 Info
    Info* pInfo = dynamic_cast<Info*>(pTarget->FindComponent(TEXT("Info")));
    if (!pInfo) return false;

    const INFO_DESC& d = pInfo->GetInfo();

    // FACTION 그룹으로 동일 그룹 타격 제외
    _int tgtFaction = 0;
    if (const InfoValue* v = d.GetPtr("Faction"))
        if (const _int* pf = std::get_if<_int>(v)) tgtFaction = *pf;

    if (tgtFaction != 0 && (_int)m_desc.faction != 0 && tgtFaction == (_int)m_desc.faction)
        return false;

    // 무적 체크
    _float invincibleLeft = 0.f;
    if (const InfoValue* v = d.GetPtr("InvincibleLeft"))
        if (const _float* pf = std::get_if<_float>(v)) invincibleLeft = *pf;

    if (invincibleLeft > 0.f)
        return false;

    // 데미지 적용
    if (Player* player = dynamic_cast<Player*>(pTarget))
    {
        _vector dirKB = XMVector3Normalize(XMVectorSetY(m_desc.moveDir, 0.f));
        const float kbPower = 3.0f;
        const float kbTime = 0.2f;
        player->SetHit(dirKB, kbPower, kbTime);

        INFO_DESC Desc = INFO_DESC{};
        Desc = pInfo->GetInfo();

        _float invLeft = 0.f;
        if (auto p = Desc.GetPtr("InvincibleLeft")) invLeft = *std::get_if<_float>(p);
        if (invLeft > 0.f)
            return false;

        _float cur = 0.f, invT = 0.15f;
        if (auto p = Desc.GetPtr("CurHP"))            cur = *std::get_if<_float>(p);
        if (auto p = Desc.GetPtr("InvincibleTime"))   invT = *std::get_if<_float>(p);

        cur = max(0.f, cur - m_desc.damageAmount);
        Desc.SetData("CurHP", _float{ cur });
        Desc.SetData("IsHit", _bool{ true });
        Desc.SetData("InvincibleLeft", _float{ invT });

        if (auto pT = Desc.GetPtr("Time"))
            Desc.SetData("LastHit", *std::get_if<_float>(pT));

        {
            BloodHitEffect::BLOODHITEFFECT_DESC desc{};
            desc.fLifeTime = 0.25f;
            desc.bLoop = false;
            desc.bAutoKill = true;
            desc.fRotationPerSec = 0.f;
            desc.fSpeedPerSec = 0.f;
            XMStoreFloat3(&desc.vCenterWS, static_cast<Transform*>(pTarget->FindComponent(TEXT("Transform")))->GetState(MATRIXROW_POSITION));
            desc.baseColor = _float4(1.0f, 0.2f, 0.2f, 1.f);

            m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("BloodHitEffect"), SCENE::GAMEPLAY, TEXT("Effect"), &desc);
        }

        return true;
    }
    return false;
}

void Worm_Projectile::HitProjectile(vector<Object*>& out)
{
    Collision* pMyCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    //결과값 초기화
    out.clear();

    //객체 가져오기
    vector<Object*> pObjects = m_pEngineUtility->GetAllObjects(m_pEngineUtility->GetCurrentSceneId());

    //조건 루프
    for (auto& object : pObjects)
    {
        if (object == nullptr || object->IsDead() || object == this)
            continue;

        Collision* pOtherCol = dynamic_cast<Collision*>(object->FindComponent(TEXT("Collision")));
        if (!pOtherCol)
            continue;

        if (pMyCollision->Intersect(pOtherCol))
            out.push_back(object);
    }
}
