#pragma once
#ifndef __TIMER_MANAGER_H__

#include "Manager.h"

struct TimerData
{
	TimerHandle handle;
    std::function<void(void)> Timerfunction;

	bool loop;
	float rate;

	TimerData()
		: handle{ NULL }
		, Timerfunction{ nullptr }
		, loop{ false }
		, rate{ 1.f }
	{

	}
};

class MWorld;
class ENGINE_DLL MTimerManager : public Manager<MTimerManager>
{
	//--------------------------------------------------
public:
	explicit MTimerManager();
	virtual ~MTimerManager() = default;

public:
	virtual void Tick();

public:
    Time GetCurrent() const;

public:
	const bool SetTimer(TimerHandle &handle, const std::function<void(void)>& Infunction, const bool loop, const float rate);

	const Time GetDeltaTime() const;
	const Time GetTotalTime() const;
	const Time GetPauseTime() const;
	const Time GetActiveTime() const;

	//--------------------------------------------------
	std::unordered_map<TimerHandle, std::shared_ptr<TimerData>> m_timerMap;

	TimeClock m_beginCount;
	TimeClock m_previousCount;
	TimeClock m_currentCount;
	//TimeClock m_deltaCount;

private:
	std::weak_ptr<MWorld> m_pOwningGame;
};

#define __TIMER_MANAGER_H__
#endif