#include "Include.h"
#include "FrameManager.h"

#include "TimerManager.h"
#include "MainGame.h"

FrameManager::FrameManager(std::shared_ptr<MainGame> pMainGame)
	: Manager<FrameManager>()
	, m_pOwningGame{ pMainGame }
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
	assert(m_pOwningGame.lock() != nullptr);

	m_time += m_pOwningGame.lock()->getTimerManager()->GetDeltaTime();
	m_lock = true;

	if (m_time > m_timePerFrame)
	{
		m_time = 0.f;
		m_lock = false;

		CaculateFrame(m_pOwningGame.lock()->getTimerManager()->GetTotalTime());
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
