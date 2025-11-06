#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

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

    _uint FindAnimIndex(string animName, _uint fallback = 0) const;
    _uint FindPriorityIndex(string toStateName, _uint fallback = 0) const;

    static Drone* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void Free() override;

private:
    HRESULT ReadyComponents();

    void SetUpAnimIndexMap();
    void SetUpPriorityIndexMap();
    void SetUpAIController();
    HRESULT SetUpStateMachine();

    void Move(_float fTimeDelta);

private:
    unordered_map<string, _uint> m_animIndexMap;
    unordered_map<string, _uint> m_priorityIndexMap;
    _vector m_moveDir = {};
    _bool   m_isMove = false; 
    _bool   m_isSprint = false;
};

NS_END
