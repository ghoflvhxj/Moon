#pragma once
#ifndef __FRAME_MANAGER_H__

#include "Manager.h"

class MainGame;

class ENGINE_DLL FrameManager : public Manager<FrameManager>
{
public:
	explicit FrameManager(std::shared_ptr<MainGame> pMainGame);
	virtual ~FrameManager();

public:
	void Tick();
	void CaculateFrame(const Time currentTime);

public:
	void SetTargetFrame(const Frame frame);
private:	
	Frame m_targetFrame;

public:
	const Frame GetFrame() const;
	const bool IsLock() const;
private:
	Frame m_currentFrame;
	Frame m_frameCounter;

	Time m_timePerFrame;
	Time m_time;

	Time m_elapsedTime;

	bool m_lock;

private:
	std::weak_ptr<MainGame> m_pOwningGame;
};

#define __FRAME_MANAGER_H__
#endif