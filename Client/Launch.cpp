#include "Include.h"

#include "MoonEngine.h"
#include "EngineException.h"

#include "Window.h"
#include "WindowManager.h"

#include "Renderer.h"

#include "MyGame.h"

#include "MainGameSetting.h"

#include "backends/imgui_impl_win32.h"

LPCWSTR title = TEXT("ShootingGame");
HWND g_hWnd;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	#ifdef UNICODE
	#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
	#else
	#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
	#endif

	std::shared_ptr<Window> pWindow = nullptr;
	try
	{
		auto pWindowManager = std::make_shared<WindowManager>(hInstance);
		WNDCLASS wndClass = { 0, };
		wndClass.lpfnWndProc = WndProc;
		wndClass.lpszClassName = title;
		wndClass.hInstance = hInstance;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hbrBackground = (HBRUSH)GetStockObject((int)WHITE_BRUSH);
		wndClass.hCursor = LoadCursor(0, IDC_ARROW);
		wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		wndClass.style = CS_VREDRAW | CS_HREDRAW;
		wndClass.lpszMenuName = nullptr;
		pWindowManager->AddWindowClass(wndClass);

		RECT rt = { 0, 0, getSetting()->getResolutionWidth(), getSetting()->getResolutionHeight() };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, FALSE);

		pWindow = pWindowManager->CreateWindow(title, rt.right - rt.left, rt.bottom - rt.top, title);
		g_hWnd = pWindow->getHandle();

		EngineInit(hInstance, pWindow);
		setGame(std::make_shared<MyGame>());
	}
	catch (const EngineException &e)
	{
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK);
		return 0;
	}

	MSG msg = { 0, };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (!EngineLoop())
			{
				continue;
			}

			std::wostringstream out;
			out << getMainGame()->getFrame();
			SetWindowText(pWindow->getHandle(), out.str().c_str());
		}
	}

	EngineRelease();

	return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
