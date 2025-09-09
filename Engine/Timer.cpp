#include "Timer.h"
#include "TimeManager.h"

Timer::Timer()
{
}

_float Timer::Get_TimeDelta() const
{
	return m_fTimeDelta; 
}

HRESULT Timer::Ready_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);
	QueryPerformanceCounter(&m_LastTime);
	QueryPerformanceCounter(&m_FixTime);

	QueryPerformanceFrequency(&m_CpuTick); // cpu tick
	
	return S_OK;
}

void Timer::Update_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);

	if (m_FrameTime.QuadPart - m_FixTime.QuadPart >= m_CpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&m_CpuTick);
		m_FixTime = m_FrameTime;
	}


	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart) / static_cast<_float>(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;
}

Timer* Timer::Create()
{
	Timer* pInstance = new Timer;

	if (FAILED(pInstance->Ready_Timer()))
	{
		Engine::Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void Timer::Free()
{
	__super::Free();
}
