#pragma once

#include "Include.h"
#include "Core/Delegate.h"
#include "Module/Module.h"
#include "TimerManager.h"
#include "FrameManager.h"

#include "Window.h"
#include "WindowManager.h"
#include "World.h"

#include "Utility/PerformanceTimer.h"

class MObject;
class MWindow;
class MWorld;
class GraphicDevice;
class MRenderer;
class MPhysicsEngine;
class WindowManager;

ENGINE_DLL std::shared_ptr<WindowManager>& GetWindowManager();

ENGINE_DLL FDelegate<void>& GetPostLoopDelegate();
ENGINE_DLL FDelegate<void>& GetLevelChangedDelegate();
ENGINE_DLL FDelegate<void>& GetRenderFinishedDelegate();
ENGINE_DLL FDelegate<void>& GetRenderStartedDelegate();

struct ENGINE_DLL FWorldRenderInfo
{
    std::shared_ptr<MWorld> SrcWorld;
    std::shared_ptr<MWindow> DstWindow;

    //FDelegate<int32 /*WindowID*/> OnRenderStartedDelegate;
};

class ENGINE_DLL MEngine
{
    std::vector<std::shared_ptr<MModule>> Modules;

public:
    template <class T>
    void CreateWindowAndWorld(const std::wstring& InTitle)
    {
        std::shared_ptr<MWindow> NewWindow = GetWindowManager()->AddWindow<T>(InTitle, 300, 300, g_hWnd, TEXT("ShootingGame"));
        NewWindow->Initialize();

        std::shared_ptr<MWorld> NewWorld = std::make_shared<MWorld>();
        NewWorld->Initialize();

        GetPostLoopDelegate().Add([this, NewWindow, NewWorld, InTitle]() {
            AddWorld(NewWorld, NewWindow);
        });
    }

public:
    void Init();
    void Release();
    void Loop();

    bool bRenderable = false;

public:
    void OpenLevel(const std::wstring& InPath);

public:
    void WorldFunc(uint32 InIndex);
    void AddWorld(std::shared_ptr<MWorld> InWorld, std::shared_ptr<MWindow> InWindow);
    FDelegate<void, const FWorldRenderInfo&>& GetOnWorldAddedDelegate() { return OnWorldAdded; }
    FWorldRenderInfo GetWorldInfo(int32 InIndex);
    const std::shared_ptr<MWindow> GetWorldBoundedWindow(const MWorld* InWorld);
    uint32 GetWorldNum() const { return GetSize(WorldRenderInfos); }
    // 이름 뭐라할지 모루겟음. WorldRenderInfosQueue를 빼와서 WorldRenderInfos에 넣는 작업을 함
    void UpdateTemp();
    void RemoveWindow(uint32 InWindowID);

protected:
    // 월드를 업데이트
    void UpdateWorld(uint32 InIndex);
    // 월드를 렌더링
    void RenderWorld(uint32 InIndex);
protected:
    // 월드와 윈도우 창 바인딩 정보를 저장
    std::map<int32, FWorldRenderInfo> WorldRenderInfos;
    std::map<int32, FWorldRenderInfo> WorldRenderInfosQueue;
    FDelegate<void, const FWorldRenderInfo&> OnWorldAdded;

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

public:
    PerformanceTimer CPUProfiler;
    std::wstring CPUTime;
};

ENGINE_DLL const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<MWindow> pWindow);
ENGINE_DLL void EngineLoop();
bool FrameLock(const std::shared_ptr<MWorld>& InWorld);
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


template <class T>
std::shared_ptr<T> GetWorld()
{
    return std::static_pointer_cast<T>(GetMainWorld());
}

ENGINE_DLL std::shared_ptr<MObject> DuplicateObject(std::shared_ptr<MObject> InObject);
void RegistComponent(std::shared_ptr<class MComponent> InComponent);
void UnRegistComponent(MComponent* InComponent);

template <class T>
std::shared_ptr<T> CreateObject()
{
    const FTypeDesc* TypeDesc = T::GetTypeDescStatic();
    assert(TypeDesc);
    assert(TypeDesc->IsA<MObject>());

    return std::shared_ptr<T>(static_cast<T*>(CreateObject(TypeDesc)));
}

ENGINE_DLL MObject* CreateObject(const FTypeDesc* InTypeDesc);
ENGINE_DLL void* CreateData(const FTypeDesc* InTypeDesc);
