#include "StateMachine.h"

StateMachine::StateMachine()
	:Component{}
{
}

StateMachine::StateMachine(const StateMachine& Prototype)
	:Component{Prototype}
{
}

HRESULT StateMachine::InitializePrototype()
{
    return S_OK;
}

HRESULT StateMachine::Initialize(void* pArg)
{
    if (pArg == nullptr)
        return S_OK;

    return S_OK;
}

void StateMachine::RegisterState(string name, const StateProc& proc)
{
    m_states.emplace(std::move(name), proc);
}

_bool StateMachine::HasState(const string& name) const
{
    return m_states.find(name) != m_states.end();
}

void StateMachine::AddTransition(const std::string& fromState, const Transition& transition)
{
    m_transitions[fromState].push_back(transition);
}

void StateMachine::AddTransition(const std::string& fromState, const std::string& toState, _uint priority, std::function<bool(Object*, StateMachine*)> cond)
{
    m_transitions[fromState].push_back(Transition{ std::move(cond), toState, priority });
}

void StateMachine::AddTransitions(const std::vector<std::string>& fromStates, const Transition& transition)
{
    for (const auto& s : fromStates)
        m_transitions[s].push_back(transition);
}

void StateMachine::SetState(const std::string& name)
{
    if (!m_curState.empty() && m_curState == name)
        return;

    if (!m_curState.empty())
        CallExit();

    m_curState = name;
    m_timeInState = 0.f;
    CallEnter();
}

void StateMachine::SetStateRestart(const std::string& name)
{
    if (!m_curState.empty())
        CallExit();

    m_curState = name;
    m_timeInState = 0.f;
    CallEnter();
}

void StateMachine::Update(_float fTimeDelta)
{
    m_timeInState += fTimeDelta;

    // 현재 상태 update
    if (!m_curState.empty())
    {
        auto it = m_states.find(m_curState);
        if (it != m_states.end() && it->second.update)
            it->second.update(m_pOwner, this, fTimeDelta);
    }

    // 전이 평가
    auto jt = m_transitions.find(m_curState);
    if (jt != m_transitions.end()) {
        const Transition* best = nullptr;
        for (auto& tr : jt->second) {
            if (!tr.condition) continue;
            if (tr.condition(m_pOwner, this)) {
                if (!best || tr.priority > best->priority) {
                    best = &tr;
                }
            }
        }
        if (best) {
            if (best->nextState != m_curState)
                SetState(best->nextState);
        }
    }
}

_bool StateMachine::Dispatch(const EventData& eventData)
{
    if (m_curState.empty())
        return false;

    auto it = m_states.find(m_curState);
    if (it == m_states.end())
        return false;

    if (it->second.onEvent)
        return it->second.onEvent(m_pOwner, this, eventData);

    return false;
}

_bool StateMachine::DispatchByValue(string eventName, _float fParam, void* pParam)
{
    EventData e{ std::move(eventName), fParam, pParam };
    return Dispatch(e);
}

const string& StateMachine::GetCurState() const
{
    return m_curState;
}

_float StateMachine::GetTimeInState() const
{
    return m_timeInState;
}

void StateMachine::Clear()
{
	if (!m_curState.empty())
		CallExit();

	m_states.clear();
	m_transitions.clear();
	m_curState.clear();
	m_timeInState = 0.f;
}

StateMachine* StateMachine::Create()
{
	StateMachine* pInstance = new StateMachine();

	if (FAILED(pInstance->InitializePrototype()))
	{
		MSG_BOX("Failed to Created : StateMachine");
		SafeRelease(pInstance);
	}

	return pInstance;
}

Component* StateMachine::Clone(void* pArg)
{
	StateMachine* pInstance = new StateMachine(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : StateMachine");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void StateMachine::Free()
{
	__super::Free();
}

void StateMachine::CallEnter()
{
    auto it = m_states.find(m_curState);
    if (it != m_states.end() && it->second.enter)
        it->second.enter(m_pOwner, this);
}

void StateMachine::CallExit()
{
    auto it = m_states.find(m_curState);
    if (it != m_states.end() && it->second.exit)
        it->second.exit(m_pOwner, this);
}