#include "MoonEngine.h"

#include "Window.h"
#include "WindowManager.h"
#include "MainGameSetting.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"
#include "World.h"
#include "Module/Physics/Jolt.h"
#include "Core/ResourceManager.h"
#include "Core/ResourceLoader.h"

#include "ShaderManager.h"
#include "ShaderLoader.h"

#include "Component.h"
#include "PrimitiveComponent.h"
#include "MeshComponent.h"

#include "Utility/PerformanceTimer.h"
#include "TimerManager.h"
#include "FrameManager.h"


HINSTANCE g_hInstance;
HWND g_hWnd;

std::unique_ptr<MEngine> g_Engine = std::make_unique<MEngine>();

std::shared_ptr<MWindow> g_pMainWindow				= nullptr;
std::shared_ptr<MDirectInput> g_pDirectInput		= nullptr;
std::shared_ptr<GraphicDevice> g_pGraphicDevice		= nullptr;
std::shared_ptr<MRenderer> g_pRenderer				= nullptr;
std::shared_ptr<MPhysicsEngine> g_pPhysics			= nullptr;

std::shared_ptr<MWorld> g_World	= nullptr;

std::unique_ptr<MainGameSetting> g_pSetting = std::make_unique<MainGameSetting>();
std::unique_ptr<MResourceManager> g_ResourceManager = std::make_unique<MResourceManager>();

FDelegate<void> PostLoopDeleagate;
FDelegate<void> OnLevelChangedDelegate;
FDelegate<void> OnRenderFinishedDelegate;
FDelegate<void> OnRenderStartedDelegate;
		

const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<MWindow> pWindow)
{
	g_hInstance = hInstance;
	g_hWnd = pWindow->getHandle();

	g_pMainWindow = pWindow;

    g_pDirectInput = g_Engine->GetModule<MDirectInput>();
    g_pRenderer = g_Engine->GetModule<MRenderer>();
    g_pPhysics = g_Engine->GetModule<MPhysicsEngine>();
    g_pGraphicDevice = g_Engine->GetModule<GraphicDevice>();
    g_Engine->InitializeModules();

    g_World = std::make_shared<MWorld>();
    g_World->Initialize();

    // 그냥 파라미터 없이, 함수 내부에서 world와 윈도우를 생성하도록 하는 것은 어떤지?
    GetEngine()->AddWorld(g_World, g_pMainWindow);

    g_pMainWindow->Initialize();

	return true;
}

ENGINE_DLL void EngineLoop()
{
    uint32 Num = GetEngine()->GetWorldNum();
    for (uint32 i = 0; i < Num; ++i)
    {
        GetEngine()->WorldFunc(i);
    }

    //GetEngine()->WorldFunc(0);

    //if (FrameLock())
    //{
    //    g_World->Update();
    //    GetEngine()->UpdateModules();

    //    EngineRender();

    //    float Current = GetEngine()->TimerManager.GetCurrent();
    //    float ElapsedTimeForUpdate = Current - GetEngine()->PrevProcessTime;

    //    GetEngine()->FrameManager.SetDeltaTime(ElapsedTimeForUpdate);
    //    GetEngine()->PrevProcessTime = GetEngine()->TimerManager.GetCurrent();
    //}
}

bool FrameLock(const std::shared_ptr<MWorld>& InWorld)
{
    MTimerManager& TimerManager = GetEngine()->TimerManager;
    TimerManager.Tick();
    float Current = TimerManager.GetCurrent();

    auto& FrameManager = InWorld->getFrameManager();
    float ElapsedTime = Current - FrameManager->PrevWorkFinishedTime;

    // 월드 업데이트&렌더의 완료에 걸리는 시간이 프레임 당 시간보다 적으면
    // 일찍 작업이 끝난거니 업데이트는 기다림
    return ElapsedTime >= FrameManager->GetTimePerFrame();
}

void EngineRender()
{
    /*
    // World1 렌더링
    g_pGraphicDevice->Begin();
    OnRenderStartedDelegate.Broadcast();

    GetEngine()->RenderModules();

    OnRenderFinishedDelegate.Broadcast();
    g_pGraphicDevice->End();

    // World2 렌더링
    g_pGraphicDevice->Begin(1);
    GetEngine()->RenderModules();
    g_pGraphicDevice->End();
    */
}

ENGINE_DLL void EnginePostLoop()
{
    PostLoopDeleagate.Broadcast();
    PostLoopDeleagate.Clear();
}

const bool EngineRelease()
{
    OnRenderFinishedDelegate.Clear();
    OnRenderStartedDelegate.Clear();

	g_World.reset();

    if (g_ResourceManager)
    {
	    g_ResourceManager->Release();
    }

    if (GetEngine())
    {
        GetEngine()->ReleaseModules();
    }

    // 수동 릴리즈
    if (getGraphicDevice())
    {
        getGraphicDevice()->Release();
    }

    GetEngine().reset();

    ReleaseReflection();

	return true;
}

ENGINE_DLL std::unique_ptr<MEngine>& GetEngine()
{
    return g_Engine;
}

std::shared_ptr<GraphicDevice>& getGraphicDevice()
{
	return g_pGraphicDevice;
}

ENGINE_DLL std::shared_ptr<MWindow>& GetMainWindow()
{
    return g_pMainWindow;
}

std::shared_ptr<MRenderer>& getRenderer()
{
	return g_pRenderer;
}

std::shared_ptr<MWorld>& GetMainWorld()
{
	return g_World;
}

std::unique_ptr<MainGameSetting>& getSetting()
{
	return g_pSetting;
}

ENGINE_DLL std::shared_ptr<MPhysicsEngine>& GetPhysics()
{
    return g_pPhysics;
}

ENGINE_DLL std::shared_ptr<WindowManager>& GetWindowManager()
{
    static std::shared_ptr<WindowManager> WinMgr = std::make_shared<WindowManager>(g_hInstance);
    return WinMgr;
}

void SetModule(std::unique_ptr<MModule>&& InModule)
{
    //g_Module = std::move(InModule);
    //g_Module->Initialize();
}

void RegisterComponent(std::shared_ptr<MComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    if (getRenderer())
    {
        if (std::shared_ptr<MPrimitiveComponent>& PrimitiveComp = InComponent->CastTo<MPrimitiveComponent>())
        {
            getRenderer()->AddPrimitiveComponent(PrimitiveComp);
        }
    }

    if (std::shared_ptr<MMeshComponent> MeshComp = InComponent->CastTo<MMeshComponent>())
    {
        std::weak_ptr<MMeshComponent> WeakMeshComp = MeshComp;
        MeshComp->GetBeganPlay().Add([WeakMeshComp]() {
            GetPhysics()->AddMeshComponent(WeakMeshComp.lock());
        });
    }
}

ENGINE_DLL FDelegate<void>& GetPostLoopDelegate()
{
    return PostLoopDeleagate;
}

ENGINE_DLL FDelegate<void>& GetLevelChangedDelegate()
{
    return OnLevelChangedDelegate;
}

ENGINE_DLL FDelegate<void>& GetRenderFinishedDelegate()
{
    return OnRenderFinishedDelegate;
}

ENGINE_DLL FDelegate<void>& GetRenderStartedDelegate()
{
    return OnRenderStartedDelegate;
}

void MEngine::WorldFunc(uint32 InIndex)
{
    std::shared_ptr<MWorld>& World = WorldRenderInfos[InIndex].SrcWorld;

    if (WorldRenderInfos[InIndex].DstWindow->IsDisabled())
    {
        return;
    }

    if (FrameLock(World))
    {
        float DeltaTime = TimerManager.GetCurrent() - World->getFrameManager()->PrevWorkFinishedTime;
        World->getFrameManager()->SetDeltaTime(DeltaTime);

        UpdateWorld(InIndex);
        RenderWorld(InIndex);

        if (auto& Fm = World->getFrameManager())
        {
            World->getFrameManager()->PrevWorkFinishedTime = TimerManager.GetCurrent();
        }
    }
}

void MEngine::UpdateWorld(uint32 InIndex)
{
    const auto& WorldRenderInfo = WorldRenderInfos[InIndex];
    WorldRenderInfo.SrcWorld->Update();

    UpdateModules();
}

void MEngine::RenderWorld(uint32 InIndex)
{
    auto& _GraphicDevice = getGraphicDevice();
    if (_GraphicDevice == nullptr)
    {
        return;
    }

    const auto& WorldRenderInfo = WorldRenderInfos[InIndex];
    const auto& Window = WorldRenderInfo.DstWindow;

    _GraphicDevice->Begin(Window->GetID(), Window->GetWidth<uint32>(), Window->GetHeight<uint32>());
    GetRenderStartedDelegate().Broadcast();

    getRenderer()->RenderWorld(WorldRenderInfo.SrcWorld);
    RenderModules();

    Window->Render();

    GetRenderFinishedDelegate().Broadcast();
    _GraphicDevice->End();
}

void MEngine::AddWorld(std::shared_ptr<MWorld> InWorld, std::shared_ptr<MWindow> InWindow)
{
    if (InWorld == nullptr || InWindow == nullptr)
    {
        return;
    }

    FWorldRenderInfo NewWorldRenderInfo = {};
    NewWorldRenderInfo.SrcWorld = InWorld;
    NewWorldRenderInfo.DstWindow = InWindow;

    WorldRenderInfosQueue[InWorld->GetID()] = NewWorldRenderInfo;

    if (GetPhysics())
    {
        InWorld->GetGameStartedDelegate().Add([&]() {
            GetPhysics()->StartSimulate(InWorld.get());
        });
    }

    GetOnWorldAddedDelegate().Broadcast(NewWorldRenderInfo);
}

std::shared_ptr<MWindow>& MEngine::GetWorldBoundedWindow(const std::shared_ptr<const MWorld>& InWorld)
{
    return WorldRenderInfos[InWorld->GetID()].DstWindow;
}

void MEngine::InitializeModules()
{
    for (auto& Module : Modules)
    {
        std::wstring Str;
        if (Module->Initialize())
        {
            Str = Module->GetName() + TEXT(" 모듈 초기화 완료");
            LOG(Str);
        }
        else
        {
            Str = Module->GetName() + TEXT(" 모듈 초기화 실패!!!");
            LOG(Str);
        }
    }

    PrevProcessTime = TimerManager.GetCurrent();
}

void MEngine::UpdateModules()
{
    for (auto& Module : Modules)
    {
        Module->Update();
    }

    GetOnUpdated().Broadcast();
}

void MEngine::RenderModules()
{
    uint32 InWorldIndex = 0;
    if (getGraphicDevice())
    {
        InWorldIndex = getGraphicDevice()->GetCurrentWindowIndex();
    }

    for (auto& Module : Modules)
    {
        Module->Render(InWorldIndex);
    }
}

void MEngine::ReleaseModules()
{
    for (auto& Module : Modules)
    {
        if (Module->IsManualReleaseRequired())
        {
            continue;
        }

        std::wstring Str = Module->GetName() + TEXT(" 모듈 Release");
        LOG(Str);
        Module->Release();
    }
}
