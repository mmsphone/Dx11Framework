#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

struct GRENADE_DESC
{
    _float3 vStartPos;   // 시작 위치
    _float3 vDir;        // 던지는 방향 (플레이어 전방)
};

class Grenade final : public ObjectTemplate
{
    Grenade();
    Grenade(const Grenade& Prototype);
    virtual ~Grenade() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static  Grenade* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void    Free() override;

private:
    HRESULT ReadyComponents();
    void    Explode();         // 폭발 처리 (데미지/이펙트는 나중에)

private:
    // 포물선 이동용
    _vector m_vVelocity = XMVectorZero();  // 현재 속도
    _float  m_gravity = 9.8f;            // 중력 가속도

    // 수명/폭발
    _float  m_lifeTime = 0.f;
    _float  m_fuseTime = 1.f;
    _bool   m_exploded = false;

    // 폭발 파라미터
    _float  m_explodeRadius = 3.f;
    _float  m_explodeDamage = 80.0f;

    _float  m_scaleOffset = 1.0f;
    _float m_speedForward = 6.f;
    _float m_speedUp = 2.f;
    
    _float m_restitutionY = 0.4f;
    _float m_restitutionXZ = 0.7f;
    _float  m_airDragPerSec = 0.7f;
    _float  m_groundOffset = 0.2f;

    // ★ 넉백용 파라미터
    _float  m_kbPowerMax = 3.f;   // 중심에서의 최대 넉백 파워
    _float  m_kbMinFactor = 0.1f;   // 가장자리에서도 최소 20% 정도는 밀리게
    _float  m_kbDuration = 0.2f;  // 넉백 유지 시간
};

NS_END
