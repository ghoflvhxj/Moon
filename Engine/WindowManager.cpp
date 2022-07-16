#include "stdafx.h"

#include "WindowManager.h"
#include "Window.h"

#include "MapUtility.h"

#undef FindWindow
#undef CreateWindow

WindowManager::WindowManager(const HINSTANCE hInstance)
	: Manager<WindowManager>()
{
	g_hInstance = hInstance;

	WNDCLASS wndClass = { 0, };
	wndClass.lpfnWndProc = Window::DefaultWndProc;
	wndClass.lpszClassName = DEFAULT_CLASSNAME;
	wndClass.hInstance = g_hInstance;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject((int)WHITE_BRUSH);
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClass.style = CS_VREDRAW | CS_HREDRAW;
	wndClass.lpszMenuName = nullptr;

	if (AddWindowClass(wndClass) == false)
	{
		MessageBox(0, TEXT("오류"), TEXT("WNDCLASS 등록 실패"), MB_OK);
		exit(0);
	}
}

const bool WindowManager::AddWindowClass(const WNDCLASS &wndClass)
{
	// RegisterClass는 실패시 0을 반환함
	return RegisterClass(&wndClass);
}

const std::shared_ptr<Window> WindowManager::CreateWindow(LPCWSTR title, const int width, const int height, LPCWSTR className)
{
	auto pWindow = std::make_shared<Window>(title, width, height, className);
	return pWindow;
}

const std::shared_ptr<Window> WindowManager::GetWindow(const HWND hWnd)
{
	std::shared_ptr<Window> pWindow = nullptr;
	MapUtility::FindGet(m_windowMap, hWnd, pWindow);

	return pWindow;
}

const bool WindowManager::FindWindow(const HWND hWnd)
{
	return MapUtility::Find(m_windowMap, hWnd);
}

const bool WindowManager::AddWindow(const HWND hWnd, const std::shared_ptr<Window> pWindow)
{
	return MapUtility::FindInsert(m_windowMap, hWnd, pWindow);
}

const bool WindowManager::SetMainWindow(const std::shared_ptr<Window> pWindow)
{
	m_pMainWindow = pWindow;
	return true;
}

const std::shared_ptr<Window> WindowManager::GetMainWindow()
{
	return m_pMainWindow;
}
