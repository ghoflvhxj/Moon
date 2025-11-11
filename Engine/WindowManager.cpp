#include "Include.h"

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
	wndClass.lpfnWndProc = MWindow::DefaultWndProc;
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

const std::shared_ptr<MWindow> WindowManager::GetWindow(const HWND hWnd)
{
	std::shared_ptr<MWindow> pWindow = nullptr;
	MapUtility::FindGet(m_windowMap, hWnd, pWindow);

	return pWindow;
}

void WindowManager::FilterWindow(std::function<bool(std::shared_ptr<MWindow>)> InFilterFunc, std::vector<std::shared_ptr<MWindow>>& OutWindows)
{
    OutWindows.reserve(m_windowMap.size());

    for (auto& [Handle, Window] : m_windowMap)
    {
        if (InFilterFunc(Window))
        {
            OutWindows.push_back(Window);
        }
    }
}

bool WindowManager::FindWindow(const HWND hWnd)
{
	return MapUtility::Find(m_windowMap, hWnd);
}

bool WindowManager::AddWindow(std::shared_ptr<MWindow> pWindow)
{
    HWND Handle = pWindow->getHandle();
    if (MapUtility::FindInsert(m_windowMap, Handle, pWindow))
    {
        pWindow->SetID(WindowIDCounter++);
        return true;
    }

	return false;
}

const bool WindowManager::SetMainWindow(const std::shared_ptr<MWindow> pWindow)
{
	m_pMainWindow = pWindow;
	return true;
}

const std::shared_ptr<MWindow> WindowManager::GetMainWindow()
{
	return m_pMainWindow;
}
