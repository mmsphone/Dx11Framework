#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Engine)
struct tagAIInputDesc;
class Info;
NS_END

NS_BEGIN(Client)

class Drone final : public ObjectTemplate
{
private:
    Drone();
    Drone(const Drone& Prototype);
    virtual ~Drone() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;    
    virtual void Update(_float fTimeDelta) override;
    virtual void LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT RenderShadow(_uint iIndex) override;

    _uint FindAnimIndex(string animName, _uint fallback = 0) const;
    _uint FindPriorityIndex(string toStateName, _uint fallback = 0) const;

    static Drone* Create();
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
    void Attack();
    void ApplyDamageToPlayer(Info* info, _float damage);

private:
    unordered_map<string, _uint> m_animIndexMap;
    unordered_map<string, _uint> m_priorityIndexMap;

    tagAIInputDesc*    m_pAIInputCache = nullptr;
    _bool   m_isMove = false;
    _vector m_aimDir = {};
    _bool   m_isSprint = false;
    _bool   m_isAttack = false;

    _bool   m_targetAttackable = true;
};

NS_END
