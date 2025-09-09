#pragma once

#include "Base.h"

NS_(Engine)

class Timer;
class TimeManager final : public Base
{
private:
	TimeManager();
	virtual ~TimeManager() = default;

public:
	_float			Get_TimeDelta(const _wstring& pTimerTag);
	HRESULT			Add_Timer(const _wstring& pTimerTag);
	void			Update_TimeDelta(const _wstring& pTimerTag);

	static TimeManager* Create();
	virtual void		Free() override;

private:
	Timer* Find_Timer(const _wstring& pTimerTag);

private:
	map<const _wstring, Timer*>			m_Timers;
};

_NS