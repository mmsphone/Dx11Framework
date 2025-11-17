#pragma once

#include "Client_Defines.h"
#include "EffectTemplate.h"

class BloodHitEffect final : public EffectTemplate
{
public:
    typedef struct tagBloodHitEffect final : public Effect::EFFECT_DESC
    {
        _float3 vCenterWS = _float3(0.f, 0.f, 0.f);
        _float4 baseColor = _float4(1.f, 1.f, 1.f, 1.f);
    }BLOODHITEFFECT_DESC;

private:
    BloodHitEffect();
    BloodHitEffect(const BloodHitEffect& Prototype);
    virtual ~BloodHitEffect() = default;

public:
    virtual HRESULT Render() override;

    static BloodHitEffect* Create();
    virtual Engine::Object* Clone(void* pArg) override;
    virtual void           Free() override;
        
private:
    virtual HRESULT ReadyComponents() override;
    virtual void    OnEffectStart(void* pArg) override;
    virtual void    OnEffectUpdate(_float fTimeDelta) override;
    virtual void OnEffectEnd() override;

private:
    BLOODHITEFFECT_DESC m_desc{};
};
