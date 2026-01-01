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
        if (!bRecorded)
        {
            std::chrono::duration<double> sec = std::chrono::system_clock::now() - _start;
            std::wstring timeLog = _name + TEXT(" ") + std::to_wstring(sec.count()) + TEXT("\r\n");
            OutputDebugStringW(timeLog.c_str());
        }
    }

    void Start()
    {
        bRecorded = false;
        _start = std::chrono::system_clock::now();
    }

    std::wstring Record()
    {
        bRecorded = true;
        std::chrono::duration<double> sec = std::chrono::system_clock::now() - _start;
        std::wstring timeLog = _name + TEXT(" ") + std::to_wstring(sec.count()) + TEXT("\r\n");
        return timeLog;
    }

    std::wstring _name;
    std::chrono::system_clock::time_point _start;
    bool bRecorded = false;
};
