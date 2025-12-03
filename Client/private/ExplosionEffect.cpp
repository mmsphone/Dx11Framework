#include "ExplosionEffect.h"

#include "EngineUtility.h"

ExplosionEffect::ExplosionEffect()
    : EffectTemplate{}
{
}

ExplosionEffect::ExplosionEffect(const ExplosionEffect& Prototype)
    : EffectTemplate{ Prototype }
    , m_desc{ Prototype.m_desc }
    , m_layers{ Prototype.m_layers }
{
}

HRESULT ExplosionEffect::ReadyComponents()
{
    // 폭발 전용 셰이더
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_Explosion"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    // ─── InstancingRect VIBuffer들 ───
    // 큰 판 1장
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_Explosion"), TEXT("VB_Sprite"), nullptr, nullptr)))
        return E_FAIL;

    // 파티클 / Fire / Smoke / Debris
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionParticle"), TEXT("VB_Particle"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionFire"), TEXT("VB_Fire"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionSmoke"), TEXT("VB_Smoke"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_ExplosionDebris"), TEXT("VB_Debris"), nullptr, nullptr)))
        return E_FAIL;

    // ─── 폭발용 텍스처 전부 컴포넌트로 붙이기 ───
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionComet"), TEXT("Tex_Comet"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionParticle"), TEXT("Tex_Particle"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionFire"), TEXT("Tex_Fire"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionBlast"), TEXT("Tex_Blast"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionDebris1"), TEXT("Tex_Debris1"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionDebris2"), TEXT("Tex_Debris2"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow1"), TEXT("Tex_Glow1"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow2"), TEXT("Tex_Glow2"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow3"), TEXT("Tex_Glow3"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionGlow4"), TEXT("Tex_Glow4"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionRing"), TEXT("Tex_Ring"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionSmoke"), TEXT("Tex_Smoke"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_ExplosionCrater"), TEXT("Tex_Crater"), nullptr, nullptr)))
        return E_FAIL;

    SetUpTextureData();
    return S_OK;
}

void ExplosionEffect::SetUpTextureData()
{
    m_layers.clear();
    m_layers.reserve(8); // 대충 여유 있게

    // ★ 여기서부터는 "기본 틀"만 잡아줌.
    //    tilesX / tilesY / totalFrames 는 전부 1로 넣어뒀으니
    //    실제 스프라이트 시트 구조 보고 직접 수정하면 됨.

    // 1) 초기 폭발 코어 (강한 불빛 / 불꽃 느낌) - Fire
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Fire");
        L.startNorm = 0.0f;
        L.endNorm = 0.35f;
        L.startScaleMul = 0.5f;
        L.endScaleMul = 1.2f;
        L.color = _float4(1.f, 0.9f, 0.7f, 1.f); // 따뜻한 색
        L.vbType = VB_TYPE::FIRE;

        // TODO: fire 스프라이트 시트 사용 시 실제 타일 수/프레임 수로 수정
        L.tilesX = 10;
        L.tilesY = 10;
        L.totalFrames = 95;

        m_layers.push_back(L);
    }

    // 2) 폭발 쇼크웨이브 (블라스트/링 웨이브)
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Blast");
        L.startNorm = 0.05f;
        L.endNorm = 0.45f;
        L.startScaleMul = 0.8f;
        L.endScaleMul = 1.6f;
        L.color = _float4(1.f, 1.f, 1.f, 0.9f);
        L.vbType = VB_TYPE::SPRITE;

        // TODO: fluid_blast 스프라이트 시트 정보
        L.tilesX = 8;
        L.tilesY = 4;
        L.totalFrames = 23;

        m_layers.push_back(L);
    }

    // 3) 링 웨이브 (particle_ring_wave_5)
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Ring");
        L.startNorm = 0.1f;
        L.endNorm = 0.6f;
        L.startScaleMul = 1.0f;
        L.endScaleMul = 2.0f;
        L.color = _float4(0.9f, 0.9f, 1.f, 0.8f);
        L.vbType = VB_TYPE::SPRITE;

        // TODO: ring 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;
        L.isFacingUp = true;
        m_layers.push_back(L);
    }

    // 4) 글로우 (중심부 밝은 빛) - Glow1~4 섞어서 여러 장
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Glow1");
        L.startNorm = 0.0f;
        L.endNorm = 0.5f;
        L.startScaleMul = 0.7f;
        L.endScaleMul = 1.5f;
        L.color = _float4(1.f, 0.85f, 0.6f, 1.f);
        L.vbType = VB_TYPE::SPRITE;

        // TODO: glow1 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Glow2");
        L.startNorm = 0.0f;
        L.endNorm = 0.6f;
        L.startScaleMul = 1.0f;
        L.endScaleMul = 1.8f;
        L.color = _float4(1.f, 0.7f, 0.4f, 0.8f);
        L.vbType = VB_TYPE::SPRITE;

        // TODO: glow2 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Glow3");
        L.startNorm = 0.1f;
        L.endNorm = 0.7f;
        L.startScaleMul = 1.2f;
        L.endScaleMul = 2.2f;
        L.color = _float4(1.f, 0.5f, 0.3f, 0.6f);
        L.vbType = VB_TYPE::SPRITE;

        // TODO: glow3 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Glow4");
        L.startNorm = 0.15f;
        L.endNorm = 0.8f;
        L.startScaleMul = 1.3f;
        L.endScaleMul = 2.5f;
        L.color = _float4(0.9f, 0.6f, 0.3f, 0.5f);
        L.vbType = VB_TYPE::SPRITE;

        // TODO: glow4 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }

    // 5) 파티클/코멧 (날아가는 파편 느낌)
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Particle");
        L.startNorm = 0.0f;
        L.endNorm = 0.4f;
        L.startScaleMul = 0.6f;
        L.endScaleMul = 1.3f;
        L.color = _float4(1.f, 0.8f, 0.6f, 0.9f);
        L.vbType = VB_TYPE::PARTICLE;

        // TODO: fire_particle_7 스프라이트 시트 정보
        L.tilesX = 16;
        L.tilesY = 4;
        L.totalFrames = 54;

        m_layers.push_back(L);
    }
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Comet");
        L.startNorm = 0.0f;
        L.endNorm = 0.5f;
        L.startScaleMul = 0.7f;
        L.endScaleMul = 1.4f;
        L.color = _float4(1.f, 0.8f, 0.7f, 0.9f);
        L.vbType = VB_TYPE::PARTICLE;

        // TODO: comet 스프라이트 시트 정보
        L.tilesX = 8;
        L.tilesY = 4;
        L.totalFrames = 22;

        m_layers.push_back(L);
    }

    // 6) 연기
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Smoke");
        L.startNorm = 0.2f;
        L.endNorm = 1.0f;
        L.startScaleMul = 1.0f;
        L.endScaleMul = 2.8f;
        L.color = _float4(0.4f, 0.4f, 0.4f, 0.8f);
        L.vbType = VB_TYPE::SMOKE;

        // TODO: smoke1 스프라이트 시트 정보
        L.tilesX = 6;
        L.tilesY = 1;
        L.totalFrames = 6;

        m_layers.push_back(L);
    }

    // 7) Debris (지금 구조로는 "큰 스프라이트" 한 장으로만 사용)
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Debris1");
        L.startNorm = 0.05f;
        L.endNorm = 0.5f;
        L.startScaleMul = 1.0f;
        L.endScaleMul = 2.0f;
        L.color = _float4(0.7f, 0.7f, 0.7f, 0.8f);
        L.vbType = VB_TYPE::DEBRIS;

        // TODO: particle_debris_burst_001 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Debris2");
        L.startNorm = 0.05f;
        L.endNorm = 0.6f;
        L.startScaleMul = 1.2f;
        L.endScaleMul = 2.2f;
        L.color = _float4(0.7f, 0.7f, 0.7f, 0.8f);
        L.vbType = VB_TYPE::DEBRIS;

        // TODO: particle_debris_burst_002 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }

    // 8) 크레이터 (바닥에 남는 흔적)
    // ─ 지금 구조는 빌보드라 바닥 데칼처럼 정확하진 않지만, 일단 같이 그림
    {
        LAYER_DESC L{};
        L.szTextureComponentName = TEXT("Tex_Crater");
        L.startNorm = 0.4f;
        L.endNorm = 1.0f;
        L.startScaleMul = 1.0f;
        L.endScaleMul = 2.0f;
        L.color = _float4(0.4f, 0.4f, 0.4f, 0.7f);
        L.isFacingUp = true;
        L.vbType = VB_TYPE::SPRITE;

        // TODO: snow_crater_1 스프라이트 시트 정보
        L.tilesX = 1;
        L.tilesY = 1;
        L.totalFrames = 1;

        m_layers.push_back(L);
    }
}

void ExplosionEffect::OnEffectStart(void* pArg)
{
    m_fAccTime = 0.f;

    if (pArg)
        m_desc = *static_cast<EXPLOSIONEFFECT_DESC*>(pArg);
    else
        m_desc = {};

    if (m_desc.fLifeTime <= 0.f)
        m_desc.fLifeTime = 0.8f;

    if (m_desc.fRadius <= 0.f)
        m_desc.fRadius = 2.0f;

    auto* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (!pTransform)
        return;

    pTransform->SetState(MATRIXROW_POSITION, XMLoadFloat3(&m_desc.vCenterWS));
    pTransform->SetScale(1.f, 1.f, 1.f);
}

void ExplosionEffect::OnEffectUpdate(_float fTimeDelta)
{
    m_fAccTime += fTimeDelta;

    // ─ InstancingRect 이동 업데이트 ─
    auto* pVB_Particle = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Particle")));
    auto* pVB_Fire = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Fire")));
    auto* pVB_Smoke = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Smoke")));
    auto* pVB_Debris = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Debris")));

    if (pVB_Particle) pVB_Particle->Spread(fTimeDelta);                 // 방사형
    if (pVB_Fire)     pVB_Fire->SpreadXZ(fTimeDelta);                   // XZ만
    if (pVB_Smoke)    pVB_Smoke->SpreadUpward(fTimeDelta);              // 위로 떠오르며 확산
    if (pVB_Debris)   pVB_Debris->SpreadGravity(fTimeDelta, 6.f);   // 중력

    if (m_fAccTime >= m_desc.fLifeTime)
        SetDead(true);
}

void ExplosionEffect::OnEffectEnd()
{
    // 특별히 정리할 건 없음 (SetDead로 관리)
}

HRESULT ExplosionEffect::Render()
{
    if (IsDead())
        return S_OK;

    auto* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    auto* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));
    auto* pVB_Sprite = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Sprite")));
    auto* pVB_Particle = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Particle")));
    auto* pVB_Fire = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Fire")));
    auto* pVB_Smoke = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Smoke")));
    auto* pVB_Debris = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VB_Debris")));

    if (!pTransform || !pShader)
        return E_FAIL;

    // 월드/뷰/프로젝
    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix",
        m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix",
        m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance",
        m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
        return E_FAIL;

    const _float life = max(m_desc.fLifeTime, 0.0001f);
    const _float tGlobal = clamp(m_fAccTime / life, 0.f, 1.f);

    for (const auto& L : m_layers)
    {
        if (!L.szTextureComponentName)
            continue;
        if (tGlobal < L.startNorm || tGlobal > L.endNorm)
            continue;

        _float tLocal = 0.f;
        if (L.endNorm > L.startNorm)
        {
            tLocal = (tGlobal - L.startNorm) / (L.endNorm - L.startNorm);
            tLocal = clamp(tLocal, 0.f, 1.f);
        }

        // 반경 스케일
        const _float scaleMul = L.startScaleMul + (L.endScaleMul - L.startScaleMul) * tLocal;
        const _float radiusScale = m_desc.fRadius * scaleMul;
        if (FAILED(pShader->BindRawValue("g_RadiusScale", &radiusScale, sizeof(_float))))
            continue;

        // 색 / 알파 페이드
        _float4 color = L.color;
        color.w *= (1.f - tLocal);
        if (FAILED(pShader->BindRawValue("g_BaseColor", &color, sizeof(_float4))))
            continue;

        // 스프라이트 시트
        _float2 tileCount = _float2(
            static_cast<_float>(max(L.tilesX, 1)),
            static_cast<_float>(max(L.tilesY, 1)));
        _float frameIndex = 0.f;
        _float useSpriteSheet = 0.f;

        const _int totalFrames = max(L.totalFrames, 1);
        if (totalFrames > 1)
        {
            frameIndex = static_cast<_float>(static_cast<_int>(tLocal * totalFrames));
            if (frameIndex >= totalFrames)
                frameIndex = static_cast<_float>(totalFrames - 1);
            useSpriteSheet = 1.f;
        }

        pShader->BindRawValue("g_TileCount", &tileCount, sizeof(_float2));
        pShader->BindRawValue("g_FrameIndex", &frameIndex, sizeof(_float));
        pShader->BindRawValue("g_UseSpriteSheet", &useSpriteSheet, sizeof(_float));

        const _float isFacingUp = L.isFacingUp ? 1.f : 0.f;
        pShader->BindRawValue("g_IsFacingUp", &isFacingUp, sizeof(_float));

        // 텍스처
        auto* pTexture = static_cast<Texture*>(FindComponent(L.szTextureComponentName));
        if (!pTexture)
            continue;
        if (FAILED(pTexture->BindRenderTargetShaderResource(pShader, "g_DiffuseTexture", 0)))
            continue;

        // 레이어에 맞는 VIBuffer 선택
        VIBufferInstancingRect* pVB = nullptr;
        switch (L.vbType)
        {
        case VB_TYPE::SPRITE:   pVB = pVB_Sprite;   break;
        case VB_TYPE::PARTICLE: pVB = pVB_Particle; break;
        case VB_TYPE::FIRE:     pVB = pVB_Fire;     break;
        case VB_TYPE::SMOKE:    pVB = pVB_Smoke;    break;
        case VB_TYPE::DEBRIS:   pVB = pVB_Debris;   break;
        default:                pVB = pVB_Sprite;   break;
        }

        if (!pVB)
            continue;

        pVB->BindBuffers();
        pShader->Begin(0);
        pVB->Render();
    }

    return S_OK;
}

ExplosionEffect* ExplosionEffect::Create()
{
    ExplosionEffect* pInstance = new ExplosionEffect();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : ExplosionEffect");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Engine::Object* ExplosionEffect::Clone(void* pArg)
{
    ExplosionEffect* pInstance = new ExplosionEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : ExplosionEffect");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void ExplosionEffect::Free()
{
    __super::Free();
}
