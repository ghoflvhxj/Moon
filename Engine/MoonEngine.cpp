#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Window.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"
#include "MainGame.h"
#include "MPhysX.h"
#include "Core/Physics/Jolt.h"

#include "ShaderManager.h"
#include "ShaderLoader.h"

#include "Core/ResourceManager.h"
#include "Core/ResourceLoader.h"

HINSTANCE g_hInstance;
HWND g_hWnd;

std::unique_ptr<MainGameSetting> g_pSetting			= std::make_unique<MainGameSetting>();
std::shared_ptr<Window> g_pMainWindow				= nullptr;
std::unique_ptr<DirectInput> g_pDirectInput			= nullptr;
std::unique_ptr<GraphicDevice> g_pGraphicDevice		= nullptr;
std::unique_ptr<MShaderManager> ShaderManager		= nullptr;
std::unique_ptr<Renderer> g_pRenderer				= nullptr;
std::unique_ptr<MainGame> g_pMainGame				= nullptr;
std::unique_ptr<MPhysics> g_pPhysics				= nullptr;
ENGINE_DLL std::unique_ptr<MResourceManager> g_ResourceManager	= nullptr;
		

const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow)
{
	g_hInstance = hInstance;
	g_hWnd = pWindow->getHandle();

	g_pMainWindow		= pWindow;

	g_pDirectInput		= std::make_unique<DirectInput>();

	g_pGraphicDevice	= std::make_unique<GraphicDevice>();
	
	ShaderManager	= std::make_unique<MShaderManager>();
	ShaderLoader shaderLoader;
	shaderLoader.loadShaderFiles(ShaderManager);

	g_pGraphicDevice->BuildInputLayout();

    //g_pPhysics = std::make_unique<MPhysX>();
    g_pPhysics = std::make_unique<MJoltPhysics>();

	g_pRenderer			= std::make_unique<Renderer>();

	g_ResourceManager = std::make_unique<MResourceManager>();
	g_ResourceManager->AddLoader(std::make_shared<MTextureLoader>());

	return true;
}

const bool EngineLoop()
{
	return g_pMainGame->Loop();
}

const bool EngineRelease()
{
	g_pMainGame.reset();

	ShaderManager->Release();
	g_ResourceManager->Release();
	g_pRenderer->Release();
	g_pGraphicDevice->Release();
    g_pPhysics->Release();

	return true;
}

std::unique_ptr<GraphicDevice>& getGraphicDevice()
{
	return g_pGraphicDevice;
}

std::unique_ptr<Renderer>& getRenderer()
{
	return g_pRenderer;
}

std::unique_ptr<MainGame>& getMainGame()
{
	return g_pMainGame;
}

std::unique_ptr<MainGameSetting>& getSetting()
{
	return g_pSetting;
}

const bool setGame(std::unique_ptr<MainGame>&& pGame)
{
	pGame->initialize();
	g_pMainGame = std::move(pGame);

	return true;
}
