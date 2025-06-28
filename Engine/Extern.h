#pragma once

#include <Windows.h>
#include <memory>
#include "Macro.h"

extern HINSTANCE g_hInstance;
extern HWND g_hWnd;

class MainGameSetting;
extern std::unique_ptr<MainGameSetting> g_pSetting;

//#ifdef ENGINE

extern std::shared_ptr<class Window> g_pMainWindow;
extern std::unique_ptr<class GraphicDevice> g_pGraphicDevice;
extern std::unique_ptr<class MShaderManager> ShaderManager;
extern std::unique_ptr<class Renderer> g_pRenderer;
extern std::unique_ptr<class MainGame> g_pMainGame;
extern std::unique_ptr<class DirectInput> g_pDirectInput;
extern std::unique_ptr<class MPhysX> g_pPhysics;
extern ENGINE_DLL std::unique_ptr<class MResourceManager> g_ResourceManager;

//#endif

