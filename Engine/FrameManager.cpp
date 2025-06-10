#include "Include.h"
#include "FrameManager.h"

#include "TimerManager.h"
#include "MainGame.h"

FrameManager::FrameManager(std::shared_ptr<MTimerManager>& InTimermanager)
	: Manager<FrameManager>()
	, TimerManager(InTimermanager)
	, m_targetFrame{ 0 }
	, m_currentFrame{ 0 }
	, m_frameCounter{ 0 }
	, m_timePerFrame{ 0 }
	, m_time{ 0 }
	, m_elapsedTime{ 0 }
{
	SetTargetFrame(60);
}

FrameManager::~FrameManager()
{
}

void FrameManager::Tick()
{
	if (TimerManager.expired())
	{
		return;
	}

	m_time += TimerManager.lock()->GetDeltaTime();
	m_lock = true;

	if (m_time > m_timePerFrame)
	{
		m_time = 0.f;
		m_lock = false;

		CaculateFrame(TimerManager.lock()->GetTotalTime());
	}
}

void FrameManager::CaculateFrame(const Time currentTime)
{
	++m_frameCounter;

	if (currentTime - m_elapsedTime >= 1.f)
	{
		m_elapsedTime += 1.f;

		m_currentFrame = m_frameCounter;
		m_frameCounter = 0;
	}
}

void FrameManager::SetTargetFrame(const Frame frame)
{
	m_targetFrame = frame;
	m_timePerFrame = 1.f / m_targetFrame;
}

const Frame FrameManager::GetFrame() const
{
	return m_currentFrame;
}

const bool FrameManager::IsLock() const
{
	return m_lock;
}
