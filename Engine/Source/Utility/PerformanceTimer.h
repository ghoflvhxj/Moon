#pragma once

#include "Include.h"

class PerformanceTimer
{
public:
    PerformanceTimer(const std::wstring& name = TEXT(""))
        : _start{ std::chrono::system_clock::now() }
        , _name(name)
    {

    }

    ~PerformanceTimer()
    {
        std::chrono::duration<double> sec = std::chrono::system_clock::now() - _start;
        std::wstring timeLog = _name + std::to_wstring(sec.count()) + TEXT("\r\n");
        OutputDebugStringW(timeLog.c_str());
    }

    std::wstring _name;
    std::chrono::system_clock::time_point _start;
};
