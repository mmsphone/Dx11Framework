#include "Info.h"

Info::Info()
    : Component{}
{
}

Info::Info(const Info& Prototype)
    : Component{ Prototype }
{
}

HRESULT Info::InitializePrototype()
{
    return S_OK;
}

HRESULT Info::Initialize(void* pArg)
{
    m_infoDesc.Clear();
    
    if (pArg == nullptr)
        return S_OK;
    
    return S_OK;
}
void Info::Update(_float fTimeDelta)
{
    //누적 시간 증가
    const _float* pTime = std::get_if<_float>(m_infoDesc.GetPtr("Time"));
    if(pTime != nullptr)
        m_infoDesc.SetData("Time", *pTime + fTimeDelta);
    else
        m_infoDesc.SetData("Time", fTimeDelta);

    //무적 시간 감소
    const _float* pInvincibleLeft = std::get_if<_float>(m_infoDesc.GetPtr("InvincibleLeft"));
    if(pInvincibleLeft != nullptr)
        m_infoDesc.SetData("InvincibleLeft", max(0,*pInvincibleLeft- fTimeDelta));
}

void Info::BindInfoDesc(const INFO_DESC& infoDesc)
{
    m_infoDesc = infoDesc;
}

INFO_DESC& Info::GetInfo()
{
    return m_infoDesc;
}

void Info::AddHp(_float hpAmount)
{
    const _bool isDamage = (hpAmount < 0.f);
    
    if (isDamage)
    {
        const _float invincibleLeft = *std::get_if<_float>(m_infoDesc.GetPtr("InvincibleLeft"));
        if (invincibleLeft > 0.f)
            return;
        const _float now = *std::get_if<_float>(m_infoDesc.GetPtr("Time"));
        const _float invincibleTime = *std::get_if<_float>(m_infoDesc.GetPtr("InvincibleTime"));

        m_infoDesc.SetData("LastHit", now);
        m_infoDesc.SetData("InvincibleLeft", invincibleTime);
        m_infoDesc.SetData("IsHit", true);
    }

    _float maxhp = max(0.f, *std::get_if<_float>(m_infoDesc.GetPtr("MaxHP")));
    _float curhp = clamp(*std::get_if<_float>(m_infoDesc.GetPtr("CurHP")) + hpAmount, 0.f, maxhp);
    m_infoDesc.SetData("CurHP", curhp);
    
}

_bool Info::IsDead() const
{
    return (*std::get_if<_float>(m_infoDesc.GetPtr("CurHP")) <= 0.f);
}

Info* Info::Create()
{
    Info* pInstance = new Info();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Info");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Component* Info::Clone(void* pArg)
{
    Info* pInstance = new Info(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Info");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Info::Free()
{
    __super::Free();
}