#include "stdafx.h"
#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Window.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"
#include "MainGame.h"

#include "ShaderManager.h"
#include "ShaderLoader.h"

HINSTANCE g_hInstance;
HWND g_hWnd;

std::shared_ptr<MainGameSetting> g_pSetting			= nullptr;
std::shared_ptr<Window> g_pMainWindow				= nullptr;
std::shared_ptr<DirectInput> g_pDirectInput			= nullptr;
std::shared_ptr<GraphicDevice> g_pGraphicDevice		= nullptr;
std::shared_ptr<ShaderManager> g_pShaderManager		= nullptr;
std::shared_ptr<Renderer> g_pRenderer				= nullptr;
std::shared_ptr<MainGame> g_pMainGame				= nullptr;
		

const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow)
{
	g_pSetting = std::make_shared<MainGameSetting>();

	g_hInstance = hInstance;
	g_hWnd = pWindow->getHandle();

	g_pMainWindow		= pWindow;

	g_pDirectInput		= std::make_shared<DirectInput>();

	g_pGraphicDevice	= std::make_shared<GraphicDevice>();
	
	g_pShaderManager = std::make_shared<ShaderManager>();
	ShaderLoader shaderLoader;
	shaderLoader.loadShader(g_pShaderManager);

	g_pGraphicDevice->BuildInputLayout();

	g_pRenderer			= std::make_shared<Renderer>();

	return true;
}

const bool EngineRelease()
{
	g_pShaderManager->Release();
	g_pRenderer->Release();
	g_pGraphicDevice->Release();

	return true;
}

std::shared_ptr<GraphicDevice> getGraphicDevice()
{
	return g_pGraphicDevice;
}

ENGINE_DLL std::shared_ptr<Renderer> getRenderer()
{
	return g_pRenderer;
}

const bool setGame(std::shared_ptr<MainGame> pGame)
{
	pGame->MainGame::initialize();
	g_pMainGame = pGame;

	return true;
}
