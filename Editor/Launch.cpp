#include "Launch.h"

#include "Window.h"
#include "WindowManager.h"

#include "MoonEngine.h"
#include "EngineException.h"

#include "Core/Delegate.h"

// 모듈들
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"

#include "Module/Physics/Jolt.h"


#include "Editor.h"

#include "MainGameSetting.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

#include "World.h"
#include "Utility/PerformanceTimer.h"
#include "Launch.h"


LPCWSTR title = TEXT("ShootingGame");
HWND g_hWnd;
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

	std::shared_ptr<MWindow> pWindow = nullptr;
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

        RECT rt = { 0, 0, getSetting()->getResolutionWidth<int>(), getSetting()->getResolutionHeight<int>() };
		AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, FALSE);

		pWindow = pWindowManager->CreateWindow(title, rt.right - rt.left, rt.bottom - rt.top, title);
		g_hWnd = pWindow->getHandle();

        GetEngine()->AddModule<MDirectInput>();
        GetEngine()->AddModule<GraphicDevice>();
        GetEngine()->AddModule<MJoltPhysics>();
        GetEngine()->AddModule<MRenderer>();
        GetEngine()->AddModule<MEditor>();

        EngineInit(hInstance, pWindow);

        if (getGraphicDevice())
        {
            IMGUI_CHECKVERSION();
            
            Context = ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.Fonts->AddFontFromFileTTF("Resources/Fonts/NanumSquareRoundR.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());

            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(g_hWnd);
            ImGui_ImplDX11_Init(getGraphicDevice()->getDevice(), getGraphicDevice()->getContext());

            GetRenderStartedDelegate().Add([]() {
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
            });

            GetRenderFinishedDelegate().Add([]() {
                ImGui::Render();
                ImGui::EndFrame();
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            });

            bImGuiInitialized = true;
        }

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

        EngineLoop();
        EnginePostLoop();
	}

    if (bImGuiInitialized)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(Context);
    }

	EngineRelease();

	return 0;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{    
    if (hWnd != g_hWnd)
    {
        std::cout << hWnd << std::endl;
    }

    if (bImGuiInitialized && hWnd == g_hWnd && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;


	switch (msg)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
        if (hWnd == g_hWnd)
        {
		    ::PostQuitMessage(0);
        }
        else
        {
            return true;
        }
		break;
    case WM_CLOSE:
    {
        int a = 0;
    }
        break;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

std::shared_ptr<WindowManager>& GetWindowManager()
{
    static std::shared_ptr<WindowManager> WinMgr = std::make_shared<WindowManager>(g_hInstance);
    return WinMgr;
}

