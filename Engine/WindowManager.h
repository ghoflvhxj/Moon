#pragma once
#ifndef __WINDOW_MANAGER_H__

#include "Manager.h"

#undef CreateWindow
#undef FindWindow

class MWindow;

class ENGINE_DLL WindowManager : public Manager<WindowManager>
{
	using WindowMap = std::unordered_map<HWND, std::shared_ptr<MWindow>>;

	//---------------------------------------------------------------------------------------------------------------------------------
public:
	explicit WindowManager(const HINSTANCE hInstance);
	virtual ~WindowManager() = default;

public:
	const bool AddWindowClass(const WNDCLASS &wndClass);

public:		
    template <class T>
    const std::shared_ptr<T> CreateWindow(LPCWSTR title, const int width, const int height, LPCWSTR className = DEFAULT_CLASSNAME)
    {
        if (T::GetTypeDescStatic()->IsA<MWindow>() == false)
        {
            return nullptr;
        }

        auto pWindow = std::make_shared<T>(title, width, height, className);
        AddWindow(pWindow);
        return pWindow;
    }

    template <class T>
    const std::shared_ptr<T> CreateWindow(LPCWSTR title, const int width, const int height, HWND Parent, LPCWSTR className = DEFAULT_CLASSNAME)
    {
        if (T::GetTypeDescStatic()->IsA<MWindow>() == false)
        {
            return nullptr;
        }

        auto pWindow = std::make_shared<T>(title, width, height, className);
        AddWindow(pWindow);
        return pWindow;
    }

	const std::shared_ptr<MWindow> GetWindow(const HWND hWnd);
	const bool FindWindow(const HWND hWnd);
	const bool AddWindow(std::shared_ptr<MWindow> pWindow);
private:
	WindowMap m_windowMap;
    uint32 WindowIDCounter = 0;

public:
	const bool SetMainWindow(const std::shared_ptr<MWindow> pWindow);
	const std::shared_ptr<MWindow> GetMainWindow();
private:
	std::shared_ptr<MWindow> m_pMainWindow;

};



#define __WINDOW_MANAGER_H__
#endif