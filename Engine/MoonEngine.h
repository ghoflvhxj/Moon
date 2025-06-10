#pragma once
#ifndef __MOON_ENGINE_H__

class Window;
class MainGame;
class GraphicDevice;
class Renderer;
class PhysXX;

ENGINE_DLL const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow);
ENGINE_DLL const bool EngineLoop();
ENGINE_DLL const bool EngineRelease();
ENGINE_DLL std::unique_ptr<GraphicDevice>& getGraphicDevice();
ENGINE_DLL std::unique_ptr<Renderer>& getRenderer();
ENGINE_DLL std::unique_ptr<MainGame>& getMainGame();
ENGINE_DLL std::unique_ptr<MainGameSetting>& getSetting();

template <class T>
const bool createMainGame(std::shared_ptr<T> &pGame)
{
	std::shared_ptr<T> pMainGame = std::make_shared<T>();
	pMainGame->MainGame::initialize();
	pGame = pMainGame;

	return true;
}

ENGINE_DLL const bool setGame(std::unique_ptr<MainGame>&& pGame);

#define __MOON_ENGINE_H__
#endif
