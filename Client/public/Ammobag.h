#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Ammobag : public ObjectTemplate
{
    Ammobag();
    Ammobag(const Ammobag& Prototype);
    virtual ~Ammobag() = default;

public:
    virtual void Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static Ammobag* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void Free() override;

private:
    HRESULT ReadyComponents();

    void SetVisibleAmmobagUI(_bool bVisible);
    _bool GiveAmmoToPlayer();

    _float scaleOffset = 1.5f;

    _bool  m_playerInRange = false;
    _bool  m_worked = false;   // 이미 사용했는지
    _bool  m_openedUI = false;

    // 키 반짝이는 용 타이머
    _float m_keyBlinkAcc = 0.f;
    _bool  m_keyBlinkOnState = false;

    // 드론 방식 알파 페이드용
    _bool  m_isDying = false;
    _float m_deathFade = 1.f;   // 1 → 0 으로 줄어들게
    _float m_deathTimer = 0.f;   // 누적 시간 (1초 동안 페이드)
};

NS_END
