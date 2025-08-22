#pragma once

#include "Include.h"
#include "Core/Delegate.h"

class Window;
class MainGame;
class GraphicDevice;
class Renderer;
class MPhysicsEngine;

ENGINE_DLL const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow);
ENGINE_DLL const bool EngineLoop();
ENGINE_DLL void EnginePostLoop();
ENGINE_DLL const bool EngineRelease();
ENGINE_DLL std::unique_ptr<GraphicDevice>& getGraphicDevice();
ENGINE_DLL std::unique_ptr<Renderer>& getRenderer();
ENGINE_DLL std::shared_ptr<MainGame>& getMainGame();
ENGINE_DLL std::unique_ptr<MainGameSetting>& getSetting();
ENGINE_DLL std::unique_ptr<MPhysicsEngine>& GetPhysics();
ENGINE_DLL const bool setGame(std::unique_ptr<MainGame>&& pGame);
ENGINE_DLL FDelegate<void>& GetPostLoopDelegate();
ENGINE_DLL FDelegate<void>& GetLevelChangedDelegate();

template <class T>
const bool createMainGame(std::shared_ptr<T> &pGame)
{
	std::shared_ptr<T> pMainGame = std::make_shared<T>();
	pMainGame->initialize();
	pGame = pMainGame;

	return true;
}

template <class T>
std::shared_ptr<T> GetGame()
{
    return std::static_pointer_cast<T>(getMainGame());
}



void RegisterComponent(std::shared_ptr<class Component> InComponent);


