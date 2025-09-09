#pragma once

#include "Base.h"

NS_(Engine)

class Timer final : public Base
{
private:
	Timer();
	virtual ~Timer() = default;

public:
	_float			Get_TimeDelta() const;

	HRESULT			Ready_Timer();
	void			Update_Timer();

	static			Timer* Create();
	virtual void	Free() override;
private:
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_FixTime = {};
	LARGE_INTEGER		m_LastTime = {};
	LARGE_INTEGER		m_CpuTick = {};
	_float				m_fTimeDelta = 0.f;
};

_NS