#include "Include.h"

#include "TimerManager.h"
#include "MapUtility.h"

using namespace std;
using namespace chrono;

TimerManager::TimerManager()
	: Manager<TimerManager>()
	, m_beginCount()
	, m_previousCount()
	, m_currentCount()
{
	m_beginCount = system_clock::now();

	m_previousCount = m_beginCount;
	m_currentCount = m_beginCount;
}

TimerManager::TimerManager(const std::shared_ptr<MainGame> pMainGame)
	: Manager<TimerManager>()
	, m_beginCount()
	, m_previousCount()
	, m_currentCount()
	, m_pOwningGame{ pMainGame }
{
	m_beginCount = system_clock::now();

	m_previousCount = m_beginCount;
	m_currentCount = m_beginCount;
}

void TimerManager::Tick()
{
	m_previousCount = m_currentCount;
	m_currentCount = system_clock::now();
}

const bool TimerManager::SetTimer(TimerHandle &handle, const TimerFunction &function, const bool loop, const float rate)
{
	std::shared_ptr<TimerData> newTimerData = std::make_shared<TimerData>();
	newTimerData->function = function; 
	newTimerData->loop = loop;
	newTimerData->rate = rate;

	if (MapUtility::Find(m_timerMap, handle))
	{
		m_timerMap[handle] = newTimerData;
	}
	else
	{
		handle = static_cast<TimerHandle>(m_timerMap.size());
		newTimerData->handle = handle;

		MapUtility::Insert(m_timerMap, handle, newTimerData);
	}

	return true;
}

const Time TimerManager::GetDeltaTime() const
{
	return duration<Time>(m_currentCount - m_previousCount).count();
}

const Time TimerManager::GetTotalTime() const
{
	return duration<Time>(m_currentCount - m_beginCount).count();
}

const Time TimerManager::GetPauseTime() const
{
	return Time();
}

const Time TimerManager::GetActiveTime() const
{
	return Time();
}
