#pragma once
#ifndef __TIMER_MANAGER_H__

#include "Manager.h"

using TimerFunction = std::function<void(void)>;

struct TimerData
{
	TimerHandle handle;
	TimerFunction function;

	bool loop;
	float rate;

	TimerData()
		: handle{ NULL }
		, function{ nullptr }
		, loop{ false }
		, rate{ 1.f }
	{

	}
};

class MainGame;
class ENGINE_DLL MTimerManager : public Manager<MTimerManager>
{
	//--------------------------------------------------
public:
	explicit MTimerManager();
	virtual ~MTimerManager() = default;

public:
	virtual void Tick();

public:
	const bool SetTimer(TimerHandle &handle, const TimerFunction &function, const bool loop, const float rate);

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
	std::weak_ptr<MainGame> m_pOwningGame;
};

#define __TIMER_MANAGER_H__
#endif