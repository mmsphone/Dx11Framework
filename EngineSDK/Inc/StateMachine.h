#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL StateMachine final : public Component
{
	StateMachine();
	StateMachine(const StateMachine& Prototype);
	virtual ~StateMachine() = default;
	
public:
	virtual HRESULT InitializePrototype();
	virtual HRESULT Initialize(void* pArg) override;

	void    RegisterState(string name, const StateProc& proc);
	_bool   HasState(const string& name) const;

	void    AddTransition(const std::string& fromState, const Transition& transition);
	void    AddTransition(const std::string& fromState, const std::string& toState, _uint priority, std::function<bool(Object*, StateMachine*)> cond);
	void    AddTransitions(const std::vector<std::string>& fromStates, const Transition& transition);

	void    SetState(const std::string& name);
	void    SetStateRestart(const std::string& name);

	void    Update(_float fTimeDelta);
	_bool   Dispatch(const EventData& eventData);
	_bool   DispatchByValue(string eventName, _float fParam = 0.f, void* pParam = nullptr);

	const string& GetCurState() const;
	_float GetTimeInState() const;

	void	Clear();

	static StateMachine* Create();
	Component* Clone(void* pArg) override;
	virtual void Free() override;

private:
	void            CallEnter();
	void            CallExit();

private:
	std::unordered_map<string, StateProc>                 m_states;
	std::unordered_map<string, vector<Transition>>  m_transitions;

	string m_curState;
	_float m_timeInState = { 0.f };
};

NS_END