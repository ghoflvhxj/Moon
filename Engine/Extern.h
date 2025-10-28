#pragma once

#include <Windows.h>
#include <memory>
#include "Macro.h"

extern ENGINE_DLL HINSTANCE g_hInstance;
extern ENGINE_DLL HWND g_hWnd;

class MainGameSetting;
extern std::unique_ptr<MainGameSetting> g_pSetting;

//#ifdef ENGINE

extern std::shared_ptr<class MWindow> g_pMainWindow;
extern std::shared_ptr<class GraphicDevice> g_pGraphicDevice;
extern std::shared_ptr<class MRenderer> g_pRenderer;
extern std::shared_ptr<class MWorld> g_World;
extern std::shared_ptr<class MDirectInput> g_pDirectInput;
extern std::shared_ptr<class MPhysicsEngine> g_pPhysics;

extern ENGINE_DLL std::unique_ptr<class MResourceManager> g_ResourceManager;

//#endif

