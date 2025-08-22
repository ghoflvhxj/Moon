#pragma once

#include "Include.h"
#include "Core/Delegate.h"

class MWindow;
class MWorld;
class GraphicDevice;
class Renderer;
class MPhysicsEngine;
class MModule;

ENGINE_DLL const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<MWindow> pWindow);
ENGINE_DLL void EngineLoop();
ENGINE_DLL void EnginePostLoop();
ENGINE_DLL const bool EngineRelease();
ENGINE_DLL std::unique_ptr<GraphicDevice>& getGraphicDevice();
ENGINE_DLL std::shared_ptr<MWindow>& GetMainWindow();
ENGINE_DLL std::unique_ptr<Renderer>& getRenderer();
ENGINE_DLL std::shared_ptr<MWorld>& GetMainWorld();
ENGINE_DLL std::unique_ptr<MainGameSetting>& getSetting();
ENGINE_DLL std::unique_ptr<MPhysicsEngine>& GetPhysics();
ENGINE_DLL void SetModule(std::unique_ptr<MModule>&& pGame);
ENGINE_DLL FDelegate<void>& GetPostLoopDelegate();
ENGINE_DLL FDelegate<void>& GetLevelChangedDelegate();
ENGINE_DLL FDelegate<void>& GetRenderFinishedDelegate();
ENGINE_DLL FDelegate<void>& GetRenderStartedDelegate();

template <class T>
std::shared_ptr<T> GetWorld()
{
    return std::static_pointer_cast<T>(GetMainWorld());
}



void RegisterComponent(std::shared_ptr<class Component> InComponent);


