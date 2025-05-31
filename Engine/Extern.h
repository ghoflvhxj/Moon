#pragma once
#ifndef __EXTERN_H__

#include <Windows.h>
#include <memory>

extern HINSTANCE g_hInstance;
extern HWND g_hWnd;

class MainGameSetting;
extern std::shared_ptr<MainGameSetting> g_pSetting;

#ifdef ENGINE

class Window;
extern std::shared_ptr<Window> g_pMainWindow;

class GraphicDevice;
extern std::shared_ptr<GraphicDevice> g_pGraphicDevice;

class MShaderManager;
extern std::shared_ptr<MShaderManager> ShaderManager;

class Renderer;
extern std::shared_ptr<Renderer> g_pRenderer;

class MainGame;
extern std::shared_ptr<MainGame> g_pMainGame;

class DirectInput;
extern std::shared_ptr<DirectInput> g_pDirectInput;

class PhysXX;
extern std::shared_ptr<PhysXX> g_pPhysics;

#endif

#define __EXTERN_H__
#endif