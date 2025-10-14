#pragma once

#include "Include.h"
#include "Core/Delegate.h"
#include "Module/Module.h"
#include "TimerManager.h"
#include "FrameManager.h"

class MWindow;
class MWorld;
class GraphicDevice;
class MRenderer;
class MPhysicsEngine;

class ENGINE_DLL MEngine
{
    std::vector<std::shared_ptr<MModule>> Modules;

public:
    template <class ...Args>
    void AddModules()
    {
        (([&]() {
            AddModule<Args>();
        }()), ...);
    }

    template <class T>
    std::shared_ptr<T> AddModule()
    {
        if (T::GetTypeDescStatic()->IsA<MModule>() == false)
        {
            std::wstring Str = TEXT("모듈이 아님");
            LOG(Str);
            return nullptr;
        }

        // TODO. 이미 있는지 검사는 일단 나중에 구현
        // asd

        std::shared_ptr<T> NewModule = std::make_shared<T>();
        if (NewModule == nullptr)
        {
            std::wstring Str = NewModule->GetName() + TEXT(" 모듈 생성 실패");
            LOG(Str);
            return nullptr;
        }

        Modules.push_back(NewModule);
        std::wstring Str = NewModule->GetName() + TEXT(" 모듈 생성 완료");
        LOG(Str);

        return NewModule;
    }

    template <class T>
    std::shared_ptr<T> GetModule()
    {
        if (T::GetTypeDescStatic()->IsA<MModule>() == false)
        {
            return nullptr;
        }

        for (std::shared_ptr<MModule> Module : Modules)
        {
            if (Module->IsA<T>())
            {
                return std::static_pointer_cast<T>(Module);
            }
        }

        return nullptr;
    }

public:
    MTimerManager TimerManager;
    MFrameManager FrameManager;
    float PrevProcessTime = 0.f;

public:
    FDelegate<void>& GetOnUpdated() { return OnUpdatedDelegate; }
protected:
    FDelegate<void> OnUpdatedDelegate;

public:
    void InitializeModules();
    // 모듈들의 Update를 호출함
    void UpdateModules();
    // 모듈들의 Render를 호출함
    void RenderModules();
    void ReleaseModules();
    // TODO. 단순히 모듈들을 순회해서 업데이트 호출하는 것이 아니라
    // 의존하는 모듈이 업데이트 된 후에 업데이트 해야함. 마치 톱니바퀴 처럼
    // 생각해보니 초기화도 그럴려나? 그런데 생성 순서를 사용자가 좀 만져주면 되긴 함
};

ENGINE_DLL const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<MWindow> pWindow);
ENGINE_DLL void EngineLoop();
bool FrameLock();
void EngineRender();
ENGINE_DLL void EnginePostLoop();
ENGINE_DLL const bool EngineRelease();

ENGINE_DLL std::unique_ptr<MEngine>& GetEngine();

ENGINE_DLL std::shared_ptr<GraphicDevice>& getGraphicDevice();
ENGINE_DLL std::shared_ptr<MWindow>& GetMainWindow();
ENGINE_DLL std::shared_ptr<MRenderer>& getRenderer();
ENGINE_DLL std::shared_ptr<MWorld>& GetMainWorld();
ENGINE_DLL std::shared_ptr<MPhysicsEngine>& GetPhysics();

ENGINE_DLL std::unique_ptr<MainGameSetting>& getSetting();
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


