#pragma once

#include "Client_Defines.h"
#include "EffectTemplate.h"

NS_BEGIN(Client)

class BloodDieEffect final : public EffectTemplate
{
public:
    typedef struct tagBloodDieEffectDesc final : public Effect::EFFECT_DESC
    {
        _float3 vCenterWS = _float3(0.f, 0.f, 0.f);
        _float4 baseColor = _float4(1.f, 1.f, 1.f, 1.f);
    } BLOODDIEEFFECT_DESC;

private:
    BloodDieEffect();
    BloodDieEffect(const BloodDieEffect& Prototype);
    virtual ~BloodDieEffect() = default;

public:
    virtual HRESULT Render() override;

    static BloodDieEffect* Create();
    virtual Engine::Object* Clone(void* pArg) override;
    virtual void            Free() override;

private:
    virtual HRESULT ReadyComponents() override;
    virtual void    OnEffectStart(void* pArg) override;
    virtual void    OnEffectUpdate(_float fTimeDelta) override;
    virtual void    OnEffectEnd() override;

private:
    BLOODDIEEFFECT_DESC m_desc{};
};

NS_END