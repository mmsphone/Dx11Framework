#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class Object;
class Transform;

class ENGINE_DLL AIController final : public Component
{
public:
    typedef struct tagAIControllerDesc 
    {
        float sightRange = 12.f;
        float fovDeg = 120.f;
        float attackRange = 1.8f;
        float chaseKeepSec = 0.5f;
    }AICONTROLLER_DESC;

    typedef struct tagAIOwnerDesc 
    {
        std::function<_vector()>               getOwnerPos;
        std::function<_vector()>               getOwnerLook;
        std::function<void(bool, _vector, bool)> applyInput;

        std::function<bool(_vector from, _vector to)>           hasLineOfSight;
        std::function<void(const AIBLACKBOARD_DESC&)>           debugDraw;
    }AI_OWNER_DESC;

private:
    AIController();
    AIController(const AIController& Prototype);
    virtual ~AIController() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    void Update(_float fTimeDelta);

    void BindOwnerDesc(const AI_OWNER_DESC& ownerDesc);

    static AIController* Create();
    virtual Component* Clone(void* pArg) override;
    virtual void Free() override;

private:
    void Sense(Object* owner, Transform* pTransform);
    void Decide(Object* owner, _float fTimeDelta);
    void Act(Object* owner, _float fTimeDelta);

private:
    AICONTROLLER_DESC   m_aiControllerDesc{};
    AI_OWNER_DESC       m_ownerDesc{};
    AIBLACKBOARD_DESC   m_aiBlackBoard{};

    _float m_time = 0.f;
    _float m_lastSeenTime = -999.f;
};

NS_END