#include "BloodDieEffect.h"

#include "EngineUtility.h"

BloodDieEffect::BloodDieEffect()
    : EffectTemplate{}
{
}

BloodDieEffect::BloodDieEffect(const BloodDieEffect& Prototype)
    : EffectTemplate{ Prototype }
{
}

HRESULT BloodDieEffect::Render()
{
    if (IsDead())
        return S_OK;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));

    if (!pTransform || !pShader)
        return E_FAIL;

    // 공통 행렬 / 카메라
    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;

    // 생명 시간(이펙트 전체 기준) 전달
    //_float2 lifeData = _float2(GetAccTime(), m_desc.fLifeTime);
    _float2 lifeData = _float2{ 0.f, 1.f };
    if (FAILED(pShader->BindRawValue("g_vLifeTime", &lifeData, sizeof(_float2))))
        return E_FAIL;

    // Spray 전용 UV (전체 텍스처 사용)
    _float4 uvSO = _float4(1.f, 1.f, 0.f, 0.f);
    if (FAILED(pShader->BindRawValue("g_vUVScaleOffset", &uvSO, sizeof(_float4))))
        return E_FAIL;
   
    const _float accTime = GetAccTime();
    // ----------------------------
    // 0) 중앙 Core + Sprite  (0.2초 간격으로 3번 플래시)
    // ----------------------------
    {
        const _float burstInterval = 0.2f;   // 간격
        const _uint  burstCount = 5;      // 총 3번
        const _float burstEndTime = burstInterval * burstCount;

        if (accTime < burstEndTime)
        {
            // 현재 사이클 시간 [0, burstInterval)
            const _float tCycle = fmodf(accTime, burstInterval);
            const _float norm = XMMin(1.f, tCycle / burstInterval);
            const _float flashFade = 1.f - norm;      // 지금은 더 이상 알파에 안 씀

            // 1-1) Core
            VIBufferInstancingRect* pVBCore =
                static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer1")));
            Texture* pTexCore =
                static_cast<Texture*>(FindComponent(TEXT("Texture1")));

            if (pVBCore && pTexCore)
            {
                if (FAILED(pTexCore->BindRenderTargetShaderResource(pShader, "g_MaskTex", 0)))
                    return E_FAIL;

                // ★ 알파 감소 X : 항상 고정값 사용
                _float alphaMul = 0.9f;
                _float alphaPow = 1.f;

                pShader->BindRawValue("g_vBaseColor", &m_desc.baseColor, sizeof(_float4));
                pShader->BindRawValue("g_fAlphaMul", &alphaMul, sizeof(_float));
                pShader->BindRawValue("g_fAlphaPow", &alphaPow, sizeof(_float));

                _float4 uvCore = _float4(1.f, 1.f, 0.f, 0.f);
                pShader->BindRawValue("g_vUVScaleOffset", &uvCore, sizeof(_float4));

                pVBCore->BindBuffers();
                pShader->Begin(0);
                pVBCore->Render();
            }

            // 1-2) Sprite (4프레임 아틀라스, 각 플래시마다 0→3프레임 재생)
            VIBufferInstancingRect* pVBSprite =
                static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer2")));
            Texture* pTexSprite =
                static_cast<Texture*>(FindComponent(TEXT("Texture2")));

            if (pVBSprite && pTexSprite)
            {
                if (FAILED(pTexSprite->BindRenderTargetShaderResource(pShader, "g_MaskTex", 0)))
                    return E_FAIL;

                const _float cycleRatio = XMMin(1.f, tCycle / burstInterval);   // 0 ~ 1
                const _uint  frame = XMMin<UINT>(3, static_cast<UINT>(cycleRatio * 4.f));

                _float4 uvSprite = _float4(0.25f, 1.f, 0.25f * frame, 0.f);
                pShader->BindRawValue("g_vUVScaleOffset", &uvSprite, sizeof(_float4));

                // ★ 여기도 알파 감소 X, 고정
                _float alphaMul = 0.9f;
                _float alphaPow = 1.f;

                pShader->BindRawValue("g_vBaseColor", &m_desc.baseColor, sizeof(_float4));
                pShader->BindRawValue("g_fAlphaMul", &alphaMul, sizeof(_float));
                pShader->BindRawValue("g_fAlphaPow", &alphaPow, sizeof(_float));

                pVBSprite->BindBuffers();
                pShader->Begin(0);
                pVBSprite->Render();

                // UV 복원
                uvSO = _float4(1.f, 1.f, 0.f, 0.f);
                pShader->BindRawValue("g_vUVScaleOffset", &uvSO, sizeof(_float4));
            }
        }
    }


    // ----------------------------
    // 1) 주변 튀는 피 (Spray1 / Spray2)
    //    죽는 동안 계속 튀다가 사라지는 느낌
    // ----------------------------
    VIBufferInstancingRect* pVIBufferSpray1 =
        static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer3")));
    VIBufferInstancingRect* pVIBufferSpray2 =
        static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer4")));
    Texture* pTextureSpray1 =
        static_cast<Texture*>(FindComponent(TEXT("Texture3")));
    Texture* pTextureSpray2 =
        static_cast<Texture*>(FindComponent(TEXT("Texture4")));

    auto RenderSprayGroup = [&](VIBufferInstancingRect* pVB, Texture* pTex)
        {
            if (!pVB || !pTex)
                return;

            if (FAILED(pTex->BindRenderTargetShaderResource(pShader, "g_MaskTex", 0)))
                return;

            // 시간에 따라 서서히 알파 감소 (셰이더 쪽 life-fade에 한 번 더 곱해줌)
            _float alphaMul = 0.85f;
            _float alphaPow = 1.f;

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

BloodDieEffect* BloodDieEffect::Create()
{
    BloodDieEffect* pInstance = new BloodDieEffect();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : BloodDieEffect");
        SafeRelease(pInstance);
        return nullptr;
    }

    return pInstance;
}

Engine::Object* BloodDieEffect::Clone(void* pArg)
{
    BloodDieEffect* pInstance = new BloodDieEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : BloodDieEffect");
        SafeRelease(pInstance);
        return nullptr;
    }

    return pInstance;
}

void BloodDieEffect::Free()
{
    __super::Free();
}

HRESULT BloodDieEffect::ReadyComponents()
{
    // 피 히트 이펙트랑 같은 셰이더 사용
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxBloodHit"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    // 사망용 스프레이 버퍼/텍스처 (프로토타입 이름은 원하는 대로 맞춰도 됨)
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieCore"), TEXT("VIBuffer1"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieSprite"), TEXT("VIBuffer2"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieSpray1"), TEXT("VIBuffer3"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("VIBuffer_BloodDieSpray2"), TEXT("VIBuffer4"), nullptr, nullptr)))
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

void BloodDieEffect::OnEffectStart(void* pArg)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));

    if (pArg)
    {
        m_desc = *static_cast<BLOODDIEEFFECT_DESC*>(pArg);
    }
    else
    {
        if (pTransform)
        {
            _float4x4 world = *pTransform->GetWorldMatrixPtr();
            m_desc.vCenterWS = _float3(world.m[3][0], world.m[3][1], world.m[3][2]);
        }
        // 기본 색상/수명
        m_desc.baseColor = _float4(1.f, 1.f, 1.f, 1.f);
    }

    // 기본 수명 1초 (0 이하로 들어오면 강제로 세팅)
    if (m_desc.fLifeTime <= 0.f)
        m_desc.fLifeTime = 1.0f;

    if (pTransform)
    {
        pTransform->SetState(MATRIXROW_POSITION,
            XMVectorSet(m_desc.vCenterWS.x, m_desc.vCenterWS.y, m_desc.vCenterWS.z, 1.f));

        pTransform->SetScale(1.f, 1.f, 1.f);
    }
}

void BloodDieEffect::OnEffectUpdate(_float fTimeDelta)
{
    VIBufferInstancingRect* pVIBufferSpray1 =
        static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer3")));
    VIBufferInstancingRect* pVIBufferSpray2 =
        static_cast<VIBufferInstancingRect*>(FindComponent(TEXT("VIBuffer4")));

    if (pVIBufferSpray1)
        pVIBufferSpray1->SpreadRepeat(fTimeDelta);
    if (pVIBufferSpray2)
        pVIBufferSpray2->SpreadRepeat(fTimeDelta);
}

void BloodDieEffect::OnEffectEnd()
{
}
