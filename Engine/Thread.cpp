#include "Include.h"
#include "Thread.h"

#include <process.h>

Thread::Thread(const ThreadFunction function) noexcept
	: _hThread{ NULL }
	, _threadID{ 0 }
{
	_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, function, nullptr, 0, &_threadID));
}

Thread::Thread(const ThreadFunction function, void *pParam) noexcept
	: _hThread{ NULL }
	, _threadID{ 0 }
{
	_hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, function, pParam, 0, &_threadID));
}

Thread::~Thread()
{
	CloseHandle(_hThread);
}

const HANDLE Thread::getHandle() const
{
	return _hThread;
}

const unsigned int Thread::getThreadID() const
{
	return _threadID;
}