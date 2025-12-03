#pragma once

#include "Client_Defines.h"
#include "EffectTemplate.h"

NS_BEGIN(Client)

class ExplosionEffect final : public EffectTemplate
{
public:
    // 그레네이드에서 넘겨줄 DESC
    struct EXPLOSIONEFFECT_DESC final : public Effect::EFFECT_DESC
    {
        _float3 vCenterWS = _float3(0.f, 0.f, 0.f);
        _float  fRadius = 2.0f;   // 기본 반경
    };

private:
    // 어떤 인스턴싱 버퍼를 쓸지
    enum class VB_TYPE : _int
    {
        SPRITE = 0,   // 큰 판 1장 (VIBuffer_Explosion)
        PARTICLE,     // VIBuffer_ExplosionParticle
        FIRE,         // VIBuffer_ExplosionFire
        SMOKE,        // VIBuffer_ExplosionSmoke
        DEBRIS        // VIBuffer_ExplosionDebris
    };

    // 각 텍스처 레이어별 연출 정의
    struct LAYER_DESC
    {
        // ReadyComponents에서 AddComponent 할 때 사용한 컴포넌트 이름
        const _tchar* szTextureComponentName = nullptr;

        // 이 레이어가 전체 fLifeTime 중 언제부터/언제까지 활성인지 (정규화: 0~1)
        _float  startNorm = 0.f;
        _float  endNorm = 1.f;

        // 반경(fRadius)에 곱해줄 스케일 범위 (정규화된 t 기준 선형 보간)
        _float  startScaleMul = 1.f;
        _float  endScaleMul = 1.f;

        // 레이어 기본 컬러 (텍스처 * baseColor)
        _float4 color = _float4(1.f, 1.f, 1.f, 1.f);

        // 스프라이트 시트 정보 (타일 수 / 총 프레임 수)
        _int    tilesX = 1;
        _int    tilesY = 1;
        _int    totalFrames = 1;

        _bool   isFacingUp = false;         // 링/크레이터용 (Y+ 평면)
        VB_TYPE vbType = VB_TYPE::SPRITE; // 어떤 VIBuffer를 쓸지
    };

private:
    ExplosionEffect();
    ExplosionEffect(const ExplosionEffect& Prototype);
    virtual ~ExplosionEffect() = default;

public:
    virtual HRESULT Render() override;

    static  ExplosionEffect* Create();
    virtual Engine::Object* Clone(void* pArg) override;
    virtual void            Free() override;

private:
    virtual HRESULT ReadyComponents() override;
    virtual void    OnEffectStart(void* pArg) override;
    virtual void    OnEffectUpdate(_float fTimeDelta) override;
    virtual void    OnEffectEnd() override;

    // 폭발 레이어/스프라이트 메타데이터 세팅
    void            SetUpTextureData();

private:
    EXPLOSIONEFFECT_DESC    m_desc{};

    // 여러 텍스처 레이어
    std::vector<LAYER_DESC> m_layers;
};

NS_END
