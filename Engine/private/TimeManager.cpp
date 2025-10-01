#include "TimeManager.h"
#include "Timer.h"

TimeManager::TimeManager() 
{
}

_float TimeManager::GetTimeDelta(const _wstring& pTimerTag)
{
	Timer* pTimer = FindTimer(pTimerTag);
	if (nullptr == pTimer)
		return 0.f;

	return pTimer->Get_TimeDelta();
}

HRESULT TimeManager::AddTimer(const _wstring& pTimerTag)
{
	Timer* pTimer = FindTimer(pTimerTag);

	CHECKPTR(pTimer)
		return E_FAIL;

	pTimer = Timer::Create();
	CHECKNULLPTR(pTimer)
		return E_FAIL;

	m_Timers.emplace(pTimerTag, pTimer);

	return S_OK;
}

void TimeManager::UpdateTimeDelta(const _wstring& pTimerTag)
{
	Timer* pTimer = FindTimer(pTimerTag);
	CHECKNULLPTR(pTimer)
		return;

	pTimer->Update_Timer();
}

TimeManager* TimeManager::Create()
{
	return new TimeManager();
}
void TimeManager::Free()
{
	__super::Free();

	for (auto& Pair : m_Timers)
		SafeRelease(Pair.second);

	m_Timers.clear();
}

Timer* TimeManager::FindTimer(const _wstring& pTimerTag)
{
	auto iter = m_Timers.find(pTimerTag);
	if (iter == m_Timers.end())
		return nullptr;

	return iter->second;
}