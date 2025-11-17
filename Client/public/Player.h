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

    void InputCheck();
    void Move(_float fTimeDelta);
    void Rotate(_float fTimeDelta);
    void Shoot();
    void HitBack(_float fTimeDelta);

private:
    unordered_map<string, _uint> m_animIndexMap;
    unordered_map<string, _uint> m_priorityIndexMap;
    _vector m_aimDir = {};
    _bool m_isMove = false;
    _bool m_isSprint = false;
    _bool m_isShoot = false;

    bool  m_shootAimActive = false;
    float m_shootAimElapsed = 0.f;
    float m_shootYawStart = 0.f;
    float m_shootYawTarget = 0.f;

    bool   m_kbActive = false;
    float  m_kbRemain = 0.f;
    float  m_kbDuration = 0.f;
    _vector m_kbDir = {};     // 정규화된 XZ 방향
    float   m_kbPower = 0.f;

    _float scaleOffset = 0.025f;
};

NS_END
