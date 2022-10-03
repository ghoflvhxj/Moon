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

std::shared_ptr<MainGameSetting> g_pSetting			= std::make_shared<MainGameSetting>();
std::shared_ptr<Window> g_pMainWindow				= nullptr;
std::shared_ptr<DirectInput> g_pDirectInput			= nullptr;
std::shared_ptr<GraphicDevice> g_pGraphicDevice		= nullptr;
std::shared_ptr<ShaderManager> g_pShaderManager		= nullptr;
std::shared_ptr<Renderer> g_pRenderer				= nullptr;
std::shared_ptr<MainGame> g_pMainGame				= nullptr;
		

const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow)
{
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

const bool EngineLoop()
{
	return g_pMainGame->Loop();
}

const bool EngineRelease()
{
	g_pMainGame.reset();

	g_pShaderManager->Release();
	g_pRenderer->Release();
	g_pGraphicDevice->Release();

	return true;
}

std::shared_ptr<GraphicDevice> getGraphicDevice()
{
	return g_pGraphicDevice;
}

std::shared_ptr<Renderer> getRenderer()
{
	return g_pRenderer;
}

std::shared_ptr<MainGame> getMainGame()
{
	return g_pMainGame;
}

std::shared_ptr<MainGameSetting> getSetting()
{
	return g_pSetting;
}

const bool setGame(std::shared_ptr<MainGame> pGame)
{
	pGame->MainGame::initialize();
	g_pMainGame = pGame;

	return true;
}
