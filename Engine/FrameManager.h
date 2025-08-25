#pragma once

#include "Include.h"
#include "Manager.h"

class MTimerManager;

class ENGINE_DLL MFrameManager : public Manager<MFrameManager>
{
public:
	explicit MFrameManager();
	virtual ~MFrameManager() = default;

public:
	void Tick(const MTimerManager& InTimerManager);
	void CaculateFrame(const Time currentTime);

public:
	void SetTargetFrame(const Frame frame);
private:	
	Frame m_targetFrame;

public:
	const Frame GetFrame() const;
	bool IsLocked() const;
    Time GetTimePerFrame() const { return m_timePerFrame; }

public:
    void SetDeltaTime(Time InTime);
    Time GetDeltaTime() const { return DeltaTime; }
protected:
    Time DeltaTime = 0.f;
    Time ElapsedTime = 0.f; 
    // SetDeltaTime이 호출된 횟수 누적
	uint32 Counter = 0;
    uint32 _Frame = 0.f;

private:
	//Frame m_currentFrame;

	Time m_timePerFrame;

    // 한 프레임당 시간에 도달하는 것을 감지하기 위한 누적시간 값
	Time m_time;

	

	bool m_lock;

private:
	std::weak_ptr<MTimerManager> TimerManager;
};