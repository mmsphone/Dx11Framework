#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class Timer;
class TimeManager final : public Base
{
private:
	TimeManager();
	virtual ~TimeManager() = default;

public:
	_float			GetTimeDelta(const _wstring& pTimerTag);
	HRESULT			AddTimer(const _wstring& pTimerTag);
	void			UpdateTimeDelta(const _wstring& pTimerTag);

	static TimeManager* Create();
	virtual void		Free() override;

private:
	Timer* FindTimer(const _wstring& pTimerTag);

private:
	map<const _wstring, Timer*>			m_Timers;
};

NS_END