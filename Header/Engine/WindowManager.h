#pragma once
#ifndef __WINDOW_MANAGER_H__

#include "Manager.h"

#undef CreateWindow
#undef FindWindow

class Window;

class ENGINE_DLL WindowManager : public Manager<WindowManager>
{
	using WindowMap = std::unordered_map<HWND, std::shared_ptr<Window>>;

	//---------------------------------------------------------------------------------------------------------------------------------
public:
	explicit WindowManager(const HINSTANCE hInstance);
	virtual ~WindowManager() = default;

public:
	const bool AddWindowClass(const WNDCLASS &wndClass);

public:		
	const std::shared_ptr<Window> CreateWindow(LPCWSTR title, const int width, const int height, LPCWSTR className = DEFAULT_CLASSNAME);
	const std::shared_ptr<Window> GetWindow(const HWND hWnd);
	const bool FindWindow(const HWND hWnd);
	const bool AddWindow(const HWND hWnd, const std::shared_ptr<Window> pWindow);
private:
	WindowMap m_windowMap;

public:
	const bool SetMainWindow(const std::shared_ptr<Window> pWindow);
	const std::shared_ptr<Window> GetMainWindow();
private:
	std::shared_ptr<Window> m_pMainWindow;

};



#define __WINDOW_MANAGER_H__
#endif