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

    _uint FindAnimIndex(string animName, _uint fallback = 0) const;
    _uint FindPriorityIndex(string toStateName, _uint fallback = 0) const;

    static Player* Create();
    virtual Container* Clone(void* pArg) override;
    virtual void Free() override;

protected:
    virtual HRESULT ReadyComponents() override;

    virtual HRESULT SetUpParts() override;
    void SetUpAnimIndexMap();
    void SetUpPriorityIndexMap();
    HRESULT SetUpStateMachine();

    void InputCheck();
    void Move(_float fTimeDelta);

private:
    unordered_map<string, _uint> m_animIndexMap;
    unordered_map<string, _uint> m_priorityIndexMap;
    _vector m_moveDir = {};
    _bool m_isMove = false;
    _bool m_isSprint = false;
    _bool m_isShoot = false;
};

NS_END
