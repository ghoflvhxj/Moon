#include "MoonEngine.h"

#include "Launch.h"
#include "EngineException.h"

#include "Window.h"
#include "WindowManager.h"

#include "Core/Delegate.h"

// 모듈들
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"
#include "Module/Physics/Jolt.h"
#include "Editor.h"

#include "MainGameSetting.h"
#include "World.h"
#include "Utility/PerformanceTimer.h"

#include "Editor/MainWindow.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

LPCWSTR title = TEXT("ShootingGame");
bool bImGuiInitialized = false;


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	// 콘솔 창
	#ifdef UNICODE
	#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
	#else
	#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
	#endif

    // wcout를 위한 코드.
    setlocale(LC_ALL, "");

    g_hInstance = hInstance;

    ImGuiContext* Context = nullptr;

	try
	{
		auto pWindowManager = GetWindowManager();
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

        GetEngine()->AddModule<MDirectInput>();
        GetEngine()->AddModule<GraphicDevice>();
        GetEngine()->AddModule<MJoltPhysics>();
        GetEngine()->AddModule<MRenderer>();
        GetEngine()->AddModule<MEditor>();

        auto& Window = pWindowManager->CreateWindow<MEditorMainWindow>(title, getSetting()->getResolutionWidth<int>(), getSetting()->getResolutionHeight<int>(), title);
        EngineInit(hInstance, Window);

        IMGUI_CHECKVERSION();

        GetEngine()->GetOnUpdated().Add([]() {
            std::wstring Frame = std::to_wstring(GetMainWorld()->getFrame());
            SetWindowText(g_hWnd, Frame.c_str());
        });
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
            EngineLoop();
            EnginePostLoop();
        }
	}

	EngineRelease();

	return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{    
    auto Window = GetWindowManager()->GetWindow(hWnd);
    if (Window)
    {
        if (auto EditorWindow = Window->CastTo<MEditorBaseWindow>())
        {
            if (EditorWindow->SetImGuiContext())
            {
                if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                {
                    return 1;
                }
            }
        }
    }

	switch (msg)
	{
        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
        }
	    break;
        case WM_DESTROY:
        {
            if (hWnd == g_hWnd)
            {
                ::PostQuitMessage(0);
            }
            else
            {
                Window->Disable();
            }
        }
	    break;
        case WM_CLOSE:
        {
            Window->Disable();
        }
        break;
        case WM_SIZE:
        {
        }
        break;
        case WM_EXITSIZEMOVE:
        {
            if (Window)
            {
                Window->UpdateSize();
            }
        }
        break;
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

