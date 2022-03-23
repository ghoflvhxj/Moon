#pragma once
#ifndef __THREAD_H__

class Thread
{
	using ThreadFunction = unsigned int (*)(void *pParam);

public:
	explicit Thread(const ThreadFunction function) noexcept;
	explicit Thread(const ThreadFunction function, void *pParam) noexcept;
	~Thread();

public:
	const HANDLE getHandle() const;
private:
	HANDLE _hThread;

public:
	const unsigned int getThreadID() const;
private:
	unsigned int _threadID;

};

#define __THREAD_H__
#endif