#include "Effect.h"

#include "EngineUtility.h"

Effect::Effect()
	:Object{}
{
}

Effect::Effect(const Effect& Prototype)
	:Object{Prototype}
{
}

HRESULT Effect::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

	if (pArg)
		m_EffectDesc = *static_cast<EFFECT_DESC*>(pArg);

	m_fAccTime = 0.f;
	m_bFinished = false;

	return S_OK;
}

void Effect::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (m_bFinished)
        return;

    m_fAccTime += fTimeDelta;

    // 수명 관리
    if (m_EffectDesc.fLifeTime > 0.f && m_fAccTime >= m_EffectDesc.fLifeTime)
    {
        if (m_EffectDesc.bLoop)
        {
            // 루프: 시간을 다시 0~Life 범위로 감기
            while (m_fAccTime >= m_EffectDesc.fLifeTime)
                m_fAccTime -= m_EffectDesc.fLifeTime;
        }
        else
        {
            m_bFinished = true;

            if (m_EffectDesc.bAutoKill)
                SetDead(true);
        }
    }
}

void Effect::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDER_BLEND, this);

    __super::LateUpdate(fTimeDelta);
}

const Effect::EFFECT_DESC& Effect::GetEffectDesc() const
{
    return m_EffectDesc;
}

_float Effect::GetAccTime() const
{
	return m_fAccTime;
}

_float Effect::GetLifeRatio() const
{
    if (m_EffectDesc.fLifeTime <= 0.f)
        return 0.f;

    return m_fAccTime / m_EffectDesc.fLifeTime;
}

_bool Effect::IsFinished() const
{
	return m_bFinished;
}

void Effect::Free()
{
    __super::Free();
}
