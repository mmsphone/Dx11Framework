#pragma once

#include "Client_Defines.h"
#include "ContainerTemplate.h"

NS_BEGIN(Client)

class Player final : public ContainerTemplate
{
private:
    Player();
    Player(const Player& Prototype);
    virtual ~Player() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT RenderShadow(_uint iIndex) override;

    _uint FindAnimIndex(string animName, _uint fallback = 0) const;
    _uint FindPriorityIndex(string toStateName, _uint fallback = 0) const;
    void SetHit(_vector dirXZ, float power, float duration);
    void SetPlayingMinigame(_bool bPlayingMinigame);
    void SetPickupState();

    void SetGlobalSightScale(const _float& sightScale);
    _float GetGlobalSightScale() const;

    static Player* Create();
    virtual Container* Clone(void* pArg) override;
    virtual void Free() override;

protected:
    virtual HRESULT ReadyComponents() override;

    virtual HRESULT SetUpParts() override;
    void SetUpAnimIndexMap();
    void SetUpPriorityIndexMap();
    HRESULT SetUpStateMachine();
    HRESULT SetUpInfo();
    HRESULT ReadyPlayerHitUI();

    void InputCheck();
    void Move(_float fTimeDelta);
    void Rotate(_float fTimeDelta);
    void Shoot();
    void BeginThrowAimFromMouse();
    void ThrowGrenade();
    void DoMeleeAttack();

    void HitBack(_float fTimeDelta);
    void UpdateHitUI(_float fTimeDelta);
    void UpdateAfkSound(_float fTimeDelta);

private:
    unordered_map<string, _uint> m_animIndexMap;
    unordered_map<string, _uint> m_priorityIndexMap;
    _vector m_aimDir = {};
    _bool m_isMove = false;
    _bool m_isSprint = false;
    _bool m_isShoot = false;

    _bool  m_shootAimActive = false;
    _float m_shootAimElapsed = 0.f;
    _float m_shootYawStart = 0.f;
    _float m_shootYawTarget = 0.f;

    _bool   m_kbActive = false;
    _float  m_kbRemain = 0.f;
    _float  m_kbDuration = 0.f;
    _vector m_kbDir = {};     // 정규화된 XZ 방향
    _float   m_kbPower = 0.f;

    _float scaleOffset = 0.025f;

    _vector m_hitDirWorld{};
    _float  m_hitUiRemain = 0.f;
    _float m_hitUIDuration = 0.5f;

    _bool m_isPlayingMinigame = false;
    _bool m_isReload = false;
    _bool m_isThrow = false;
    _bool m_throwed = false;
    _bool m_isPickup = false;

    _vector m_throwAimDir = XMVectorZero();
    _bool    m_hasThrowAim = false;
    _bool    m_inThrowState = false;

    _float   m_idleTime = 0.f;
    _float   m_afkSoundAcc = 0.f;
    
    _float m_globalSightScale = 1.f;

    _bool m_isMelee = false;
    _bool m_meleeDidHit = false;
};

NS_END
