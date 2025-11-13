#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Engine)
struct tagAIInputDesc;
class Info;
NS_END

NS_BEGIN(Client)

class Shieldbug final : public ObjectTemplate
{
private:
    Shieldbug();
    Shieldbug(const Shieldbug& Prototype);
    virtual ~Shieldbug() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT RenderShadow(_uint iIndex) override;

    _uint FindAnimIndex(string animName, _uint fallback = 0) const;
    _uint FindPriorityIndex(string toStateName, _uint fallback = 0) const;

    static Shieldbug* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void Free() override;

private:
    HRESULT ReadyComponents();

    void SetUpAnimIndexMap();
    void SetUpPriorityIndexMap();
    HRESULT SetUpStateMachine();
    HRESULT SetUpInfo();

    void SetUpAIInputData();
    HRESULT SetUpAIProcess();
    void Move(_float fTimeDelta);
    void Rotate(_float fTimeDelta);
    void Attack();
    void ApplyDamageToPlayer(Info* info, _float damage);

private:
    unordered_map<string, _uint> m_animIndexMap;
    unordered_map<string, _uint> m_priorityIndexMap;

    tagAIInputDesc* m_pAIInputCache = nullptr;
    _bool   m_isMove = false;
    _bool   m_isAttack = false;
    _bool   m_isDefend = false;
    _bool   m_isEnterDefend = false;
    _vector m_aimDir = {};

    _bool   m_targetAttackable = true;

    _bool  m_yawInterpActive = false;
    _float m_yawInterpT = 0.f;     // 진행 시간
    _float m_yawInterpDur = 0.2f;   // 보간 시간(튜닝)
    _float m_yawStart = 0.f;     // 시작 yaw
    _float m_yawTarget = 0.f;     // 목표 yaw
};

NS_END
