#pragma once
#ifndef __MOON_ENGINE_H__

#include "Include.h"

class Window;
class MainGame;
class GraphicDevice;
class Renderer;
class MPhysicsEngine;

ENGINE_DLL const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow);
ENGINE_DLL const bool EngineLoop();
ENGINE_DLL const bool EngineRelease();
ENGINE_DLL std::unique_ptr<GraphicDevice>& getGraphicDevice();
ENGINE_DLL std::unique_ptr<Renderer>& getRenderer();
ENGINE_DLL std::unique_ptr<MainGame>& getMainGame();
ENGINE_DLL std::unique_ptr<MainGameSetting>& getSetting();
ENGINE_DLL std::unique_ptr<MPhysicsEngine>& GetPhysics();

template <class T>
const bool createMainGame(std::shared_ptr<T> &pGame)
{
	std::shared_ptr<T> pMainGame = std::make_shared<T>();
	pMainGame->initialize();
	pGame = pMainGame;

	return true;
}

template <class T>
T* GetGame()
{
    return static_cast<T*>(getMainGame().get());
}

ENGINE_DLL const bool setGame(std::unique_ptr<MainGame>&& pGame);

void RegisterComponent(std::shared_ptr<class Component> InComponent);

#define __MOON_ENGINE_H__
#endif
