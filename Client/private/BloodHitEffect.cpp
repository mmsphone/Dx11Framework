#include "BloodHitEffect.h"

#include "EngineUtility.h"

BloodHitEffect::BloodHitEffect()
    : EffectTemplate{}
{
}

BloodHitEffect::BloodHitEffect(const BloodHitEffect& Prototype)
    : EffectTemplate{ Prototype }
{
}

HRESULT BloodHitEffect::Render()
{
    if (IsDead())
        return S_OK;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));

    if (!pTransform || !pShader)
        return E_FAIL;

    // 공통 행렬/카메라
    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;

    _float4 uvSO = _float4(1.f, 1.f, 0.f, 0.f);
    if (FAILED(pShader->BindRawValue("g_vUVScaleOffset", &uvSO, sizeof(_float4))))
        return E_FAIL;

    _float2 lifeData = _float2(GetAccTime(), m_desc.fLifeTime);
    if (FAILED(pShader->BindRawValue("g_vLifeTime", &lifeData, sizeof(_float2))))
        return E_FAIL;

    // ----------------------------
    // 1) blood_core (중앙 플래시)
    // ----------------------------
    VIBufferInstancingRect* pVIBufferCore = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer1")));
    Texture* pTextureCore = static_cast<Texture*>(FindComponent(TEXT("Texture1")));
    if (pVIBufferCore && pTextureCore)
    {
        if (FAILED(pTextureCore->BindRenderTargetShaderResource(pShader, "g_MaskTex", 0)))
            return E_FAIL;
    
        _float  alphaMul = 0.9f;
        _float  alphaPow = 1.f;
    
        pShader->BindRawValue("g_vBaseColor", &m_desc.baseColor, sizeof(_float4));
        pShader->BindRawValue("g_fAlphaMul", &alphaMul, sizeof(_float));
        pShader->BindRawValue("g_fAlphaPow", &alphaPow, sizeof(_float));
    
        pVIBufferCore->BindBuffers();
        pShader->Begin(0);
        pVIBufferCore->Render();
    }

    // ----------------------------
    // 2) blood.png (4프레임 아틀라스)
    // ----------------------------
    VIBufferInstancingRect* pVIBufferSprite = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer2")));
    Texture* pTextureSprite = static_cast<Texture*>(FindComponent(TEXT("Texture2")));
    if (pVIBufferSprite && pTextureSprite)
    {
        if (FAILED(pTextureSprite->BindRenderTargetShaderResource(pShader, "g_MaskTex", 0)))
            return E_FAIL;
    
        // Effect::GetLifeRatio 기준으로 0~3 프레임 선택
        const _float life = XMMin(1.f, GetLifeRatio());
        const _uint  frame = XMMin<UINT>(3, static_cast<UINT>(life * 4.f));
    
        uvSO.x = 0.25f;         // scaleU
        uvSO.y = 1.0f;
        uvSO.z = 0.25f * frame; // offsetU
        uvSO.w = 0.0f;
        pShader->BindRawValue("g_vUVScaleOffset", &uvSO, sizeof(_float4));
    
        _float  alphaMul = 0.9f;
        _float  alphaPow = 1.f;
    
        pShader->BindRawValue("g_vBaseColor", &m_desc.baseColor, sizeof(_float4));
        pShader->BindRawValue("g_fAlphaMul", &alphaMul, sizeof(_float));
        pShader->BindRawValue("g_fAlphaPow", &alphaPow, sizeof(_float));
    
        pVIBufferSprite->BindBuffers();
        pShader->Begin(0);
        pVIBufferSprite->Render();
    
        // UV 복원
        uvSO = _float4(1.f, 1.f, 0.f, 0.f);
        pShader->BindRawValue("g_vUVScaleOffset", &uvSO, sizeof(_float4));
    }

    // ----------------------------
    // 3) 주변 튀는 피 (Spray1 / Spray2)
    // ----------------------------
    VIBufferInstancingRect* pVIBufferSpray1 = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer3")));
    VIBufferInstancingRect* pVIBufferSpray2 = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer4")));
    Texture* pTextureSpray1 = static_cast<Texture*>(FindComponent(TEXT("Texture3")));
    Texture* pTextureSpray2 = static_cast<Texture*>(FindComponent(TEXT("Texture4")));

    auto RenderSprayGroup = [&](VIBufferInstancingRect* pVB, Texture* pTex)
        {
            if (!pVB || !pTex)
                return;

            pTex->BindRenderTargetShaderResource(pShader, "g_MaskTex", 0);

            _float  alphaMul = 0.85f;
            _float  alphaPow = 1.0f;

            pShader->BindRawValue("g_vBaseColor", &m_desc.baseColor, sizeof(_float4));
            pShader->BindRawValue("g_fAlphaMul", &alphaMul, sizeof(_float));
            pShader->BindRawValue("g_fAlphaPow", &alphaPow, sizeof(_float));

            _float4 uvAll = _float4(1.f, 1.f, 0.f, 0.f);
            pShader->BindRawValue("g_vUVScaleOffset", &uvAll, sizeof(_float4));

            pVB->BindBuffers();
            pShader->Begin(0);
            pVB->Render();
        };

    RenderSprayGroup(pVIBufferSpray1, pTextureSpray1);
    RenderSprayGroup(pVIBufferSpray2, pTextureSpray2);

    return S_OK;
}

BloodHitEffect* BloodHitEffect::Create()
{
    BloodHitEffect* pInstance = new BloodHitEffect();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : BloodHitEffect");
        SafeRelease(pInstance);
        return nullptr;
    }

    return pInstance;
}

Engine::Object* BloodHitEffect::Clone(void* pArg)
{
    BloodHitEffect* pInstance = new BloodHitEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : BloodHitEffect");
        SafeRelease(pInstance);
        return nullptr;
    }

    return pInstance;
}

void BloodHitEffect::Free()
{
    __super::Free();
}

HRESULT BloodHitEffect::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxBloodHit"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodCore"), TEXT("VIBuffer1"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSprite"), TEXT("VIBuffer2"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSpray1"), TEXT("VIBuffer3"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodSpray2"), TEXT("VIBuffer4"), nullptr, nullptr)))
        return E_FAIL;

    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_BloodCore"), TEXT("Texture1"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_BloodSprite"), TEXT("Texture2"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_BloodSpray1"), TEXT("Texture3"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Texture_BloodSpray2"), TEXT("Texture4"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

void BloodHitEffect::OnEffectStart(void* pArg)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));

    if (pArg)
        m_desc = *static_cast<BLOODHITEFFECT_DESC*>(pArg);
    else
    {
        _float4x4 world = *pTransform->GetWorldMatrixPtr();
        m_desc.vCenterWS = _float3(world.m[3][0], world.m[3][1], world.m[3][2]);
    }

    if (pTransform)
    {
        pTransform->SetState(MATRIXROW_POSITION,
            XMVectorSet(m_desc.vCenterWS.x, m_desc.vCenterWS.y, m_desc.vCenterWS.z, 1.f));
        pTransform->SetScale(1.f,1.f,1.f);
    }
}

void BloodHitEffect::OnEffectUpdate(_float fTimeDelta)
{
    VIBufferInstancingRect* pVIBufferSpray1 = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer3")));
    if (pVIBufferSpray1)
        pVIBufferSpray1->Spread(fTimeDelta);
    VIBufferInstancingRect* pVIBufferSpray2 = static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer4")));
    if (pVIBufferSpray2)
        pVIBufferSpray2->Spread(fTimeDelta);
}

void BloodHitEffect::OnEffectEnd()
{
}
