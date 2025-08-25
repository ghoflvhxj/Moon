#include "Include.h"
#include "FrameManager.h"

#include "TimerManager.h"

MFrameManager::MFrameManager()
	: Manager<MFrameManager>()
	, m_targetFrame{ 0 }
	, m_timePerFrame{ 0 }
	, m_time{ 0 }
{
	SetTargetFrame(60);
}

void MFrameManager::Tick(const MTimerManager& InTimerManager)
{
    //float delta = InTimerManager.GetDeltaTime();
    //m_time += delta;
    //m_lock = true;

    //if (m_time >= m_timePerFrame)
    //{
    //    while (m_time >= m_timePerFrame)
    //    {
    //        m_time -= m_timePerFrame;
    //    }
    //    m_lock = false;

    //    CaculateFrame(InTimerManager.GetTotalTime());
    //}
}

void MFrameManager::CaculateFrame(const Time currentTime)
{
	//++m_frameCounter;

	//if (currentTime - m_elapsedTime >= 1.f)
	//{
	//	m_elapsedTime += 1.f;

	//	m_currentFrame = m_frameCounter;
	//	m_frameCounter = 0;
	//}
}

void MFrameManager::SetTargetFrame(const Frame frame)
{
	m_targetFrame = frame;
	m_timePerFrame = 1.f / m_targetFrame;
}

const Frame MFrameManager::GetFrame() const
{
	return _Frame;
}

bool MFrameManager::IsLocked() const
{
	return m_lock;
}

void MFrameManager::SetDeltaTime(Time InTime)
{
    DeltaTime = InTime;

    ElapsedTime += DeltaTime;
    ++Counter;

    if (ElapsedTime >= 1.f)
    {
        _Frame = Counter;
        Counter = 0;
        ElapsedTime = 0.f;
    }
}
