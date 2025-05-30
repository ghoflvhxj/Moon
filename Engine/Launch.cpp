#include "Include.h"
#include "WindowManager.h"
#include "Window.h"
#include "MainGame.h"

#include "FrameManager.h"

/*
HINSTANCE g_hInstance = 0;
LPWSTR a = TEXT("A");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_hInstance = hInstance;

	WindowManager windowManager;
	auto pWindow = windowManager.CreateWindow(TEXT("My Framework"), 800, 600);

	auto pGame = std::make_shared<MainGame>();
	pGame->Initialize();

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
			if (!pGame->Loop())
				continue;

			std::wostringstream out;
			out << pGame->GetFrameManager()->GetFrame();
			SetWindowText(pWindow->GetHandle(), out.str().c_str());
		}
	}

	return 0;
}
*/