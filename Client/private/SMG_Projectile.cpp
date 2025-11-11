#include "SMG_Projectile.h"

#include "EngineUtility.h"

SMG_Projectile::SMG_Projectile()
    : Projectile{}
{
}

SMG_Projectile::SMG_Projectile(const SMG_Projectile& Prototype)
    : Projectile{ Prototype }
{
}

HRESULT SMG_Projectile::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void SMG_Projectile::Update(_float fTimeDelta)
{
    // 수명, 이동 처리 상위 클래스에서 함
    __super::Update(fTimeDelta);
    if (IsDead()) 
        return;

    // 충돌 체크
    Transform* tf = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!tf || m_desc.hitRadius <= 0.f) 
        return;

    const _vector center = tf->GetState(POSITION);

    std::vector<Object*> hits;
    HitProjectile(center, m_desc.hitRadius, hits);

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

void SMG_Projectile::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(NONBLEND, this);
    __super::LateUpdate(fTimeDelta);
}

HRESULT SMG_Projectile::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    VIBufferInstancingPoint* pVIBuffer = dynamic_cast<VIBufferInstancingPoint*>(FindComponent(TEXT("VIBuffer")));
    Texture* pTexture = dynamic_cast<Texture*>(FindComponent(TEXT("Texture")));

    if (!pTransform || !pShader || !pVIBuffer || !pTexture)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(pTexture->BindRenderTargetShaderResource(pShader, "g_Texture", 0)))
        return E_FAIL;

    pShader->Begin(0);

    pVIBuffer->BindBuffers();
    pVIBuffer->Render();

    return S_OK;
}

SMG_Projectile* SMG_Projectile::Create()
{
    SMG_Projectile* pInstance = new SMG_Projectile();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : SMG_Projectile");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* SMG_Projectile::Clone(void* pArg)
{
    SMG_Projectile* pInstance = new SMG_Projectile(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : SMG_Projectile");
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void SMG_Projectile::Free()
{
    __super::Free();
}

HRESULT SMG_Projectile::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_Particle_Bullet"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_Bullet"), TEXT("Texture"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_InstancingPos"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

_bool SMG_Projectile::TryApplyHitTo(Object* pTarget)
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
    pInfo->AddHp(-m_desc.damageAmount);
    return true;
}

void SMG_Projectile::HitProjectile(const _vector& projectilePos, _float& fHitRadius, vector<Object*>& out)
{
    //결과값 초기화
    out.clear();

    //객체 가져오기
    vector<Object*> pObjects = m_pEngineUtility->GetAllObjects(m_pEngineUtility->GetCurrentSceneId());

    //조건 루프
    const _float r2 = fHitRadius * fHitRadius;
    const _float invScaleY = 1.f / 2.f; // y축 판정만 조금 늘림 1->1.5 
    for (auto& object : pObjects)
    {
        if (object == nullptr || object->IsDead() || object == this)
            continue;

        Transform* pTransform = dynamic_cast<Transform*>(object->FindComponent(TEXT("Transform")));
        if (pTransform == nullptr)
            continue;

        const _vector delta = pTransform->GetState(POSITION) - projectilePos;

        const _float dx = XMVectorGetX(delta);
        const _float dy = XMVectorGetY(delta) * invScaleY;
        const _float dz = XMVectorGetZ(delta);

        const _float distance = dx * dx + dy * dy + dz * dz;
        if (distance <= r2)
            out.push_back(object);
    }
}
