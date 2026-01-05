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
std::unique_ptr<MFIleSystem> FileSystem = nullptr;

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

    FileSystem = std::make_unique<MFIleSystem>();

    g_pDirectInput = g_Engine->GetModule<MDirectInput>();
    g_pRenderer = g_Engine->GetModule<MRenderer>();
    g_pPhysics = g_Engine->GetModule<MPhysicsEngine>();
    g_pGraphicDevice = g_Engine->GetModule<GraphicDevice>();

    GetEngine()->Init();

    g_World = std::make_shared<MWorld>();
    g_World->Initialize();

    // 그냥 파라미터 없이, 함수 내부에서 world와 윈도우를 생성하도록 하는 것은 어떤지?
    GetEngine()->AddWorld(g_World, g_pMainWindow);

    g_pMainWindow->Initialize();

	return true;
}

ENGINE_DLL void EngineLoop()
{   
    GetEngine()->Loop();
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
    GetEngine()->UpdateTemp();

    PostLoopDeleagate.Broadcast();
    PostLoopDeleagate.Clear();
}

const bool EngineRelease()
{
    assert(GetEngine());

    OnRenderFinishedDelegate.Clear();
    OnRenderStartedDelegate.Clear();

	g_World.reset();

    GetWindowManager()->Release();

    if (g_ResourceManager)
    {
	    g_ResourceManager->Release();
        g_ResourceManager.reset();
    }

    GetEngine()->Release();

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

std::shared_ptr<MObject> DuplicateObject(std::shared_ptr<MObject> InObject)
{
    if (InObject == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<MObject> NewObject = InObject->Duplicate();

    return NewObject;
}

void RegisterComponent(std::shared_ptr<MComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    if (getRenderer())
    {
        if (std::shared_ptr<MPrimitiveComponent>& PrimitiveComp = InComponent->CastToShared<MPrimitiveComponent>())
        {
            getRenderer()->AddPrimitiveComponent(PrimitiveComp);
        }
    }

    if (std::shared_ptr<MMeshComponent> MeshComp = InComponent->CastToShared<MMeshComponent>())
    {
        std::weak_ptr<MMeshComponent> WeakMeshComp = MeshComp;
        MeshComp->GetBeganPlay().Add([WeakMeshComp]() {
            GetPhysics()->AddMeshComponent(WeakMeshComp.lock());
        });
    }

    //GetPhysics()->AddCharacterBody()
}

void UnRegisterComponent(MComponent* InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    if (getRenderer())
    {
        if (MPrimitiveComponent* PrimitiveComp = InComponent->CastTo<MPrimitiveComponent>())
        {
            getRenderer()->RemovePrimitiveComponent(PrimitiveComp);
        }
    }

    if (GetPhysics())
    {
        if (std::shared_ptr<MMeshComponent> MeshComp = InComponent->CastToShared<MMeshComponent>())
        {
            GetPhysics()->RemoveComponent(MeshComp);
        }
    }
}

ENGINE_DLL void* CreateObject(const FTypeDesc* InTypeDesc)
{
    if (GetFactory().find(InTypeDesc) != GetFactory().end())
    {
        MObject* NewObject = static_cast<MObject*>(GetFactory()[InTypeDesc]->Create());
        NewObject->PostConstruct();

        return NewObject;
    }

    return nullptr;
}

ENGINE_DLL void* CreateData(const FTypeDesc* InTypeDesc)
{
    if (GetFactory().find(InTypeDesc) != GetFactory().end())
    {
        return GetFactory()[InTypeDesc]->Create();
    }

    return nullptr;
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

void MEngine::Init()
{
    InitializeModules();

    bRenderable = getRenderer() && getGraphicDevice();
}

void MEngine::Release()
{
    WorldRenderInfos.clear();
    WorldRenderInfosQueue.clear();

    ReleaseModules();
}

void MEngine::Loop()
{
    CPUProfiler.Start();
    for (auto& [WindowID, BindData] : WorldRenderInfos)
    {
        WorldFunc(WindowID);
    }
}

void MEngine::WorldFunc(uint32 InIndex)
{
    FWorldRenderInfo& BoundData = WorldRenderInfos[InIndex];
    if (BoundData.DstWindow->IsDisabled())
    {
        return;
    }

    std::shared_ptr<MWorld>& World = BoundData.SrcWorld;
    if (FrameLock(World))
    {
        float DeltaTime = TimerManager.GetCurrent() - World->getFrameManager()->PrevWorkFinishedTime;
        World->getFrameManager()->SetDeltaTime(DeltaTime);

        UpdateWorld(InIndex);

        if (bRenderable)
        {
            RenderWorld(InIndex);
        }

        if (auto& Fm = World->getFrameManager())
        {
            World->getFrameManager()->PrevWorkFinishedTime = TimerManager.GetCurrent();
        }

        std::wstring Frame = std::to_wstring(World->getFrame());
        SetWindowText(WorldRenderInfos[InIndex].DstWindow->getHandle(), Frame.c_str());
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
    assert(getGraphicDevice());
    assert(getRenderer());

    const auto& WorldRenderInfo = WorldRenderInfos[InIndex];
    const auto& Window = WorldRenderInfo.DstWindow;

    getGraphicDevice()->Begin(Window->GetID(), Window->GetWidth<uint32>(), Window->GetHeight<uint32>());
    GetRenderStartedDelegate().Broadcast();
    getRenderer()->RenderWorld(WorldRenderInfo.SrcWorld);

    RenderModules();

    Window->Render();

    GetRenderFinishedDelegate().Broadcast();

    CPUTime = CPUProfiler.Record();
    getGraphicDevice()->End();
}

void MEngine::UpdateTemp()
{
    for (auto& Iter = WorldRenderInfosQueue.begin(); Iter != WorldRenderInfosQueue.end(); )
    {
        WorldRenderInfos.insert(*Iter);
        Iter = WorldRenderInfosQueue.erase(Iter);
    }
}

void MEngine::RemoveWindow(uint32 InWindowID)
{
    // TODO
    // 월드와 윈도우가 1:1 상황이라면, shared_ptr 레퍼런스 카운트가 감소하여 월드 또한 파괴될 것임
    // 지금은 1:1 상황밖에 없어서 임시로 처리하지만 개선해야함
    auto Erase = [](std::map<int32, FWorldRenderInfo>& InMap, uint32 InID) {
        for (auto& Iter = InMap.begin(); Iter != InMap.end(); )
        {
            auto& BoundData = Iter->second;
            if (BoundData.DstWindow->GetID() == InID)
            {
                BoundData.SrcWorld->FinishGame(); // 임시처리
                Iter = InMap.erase(Iter);
            }
            else
            {
                ++Iter;
            }
        }
    };

    GetPostLoopDelegate().Add([&, InWindowID]() {
        Erase(WorldRenderInfos, InWindowID);
        Erase(WorldRenderInfosQueue, InWindowID);
    });
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

FWorldRenderInfo MEngine::GetWorldInfo(int32 InIndex)
{
    if (WorldRenderInfos.find(InIndex) != WorldRenderInfos.end())
    {
        return WorldRenderInfos[InIndex];
    }
    else if (WorldRenderInfosQueue.find(InIndex) != WorldRenderInfosQueue.end())
    {
        return WorldRenderInfosQueue[InIndex];
    }

    return FWorldRenderInfo();
}

const std::shared_ptr<MWindow> MEngine::GetWorldBoundedWindow(const MWorld* InWorld)
{
    uint32 ID = InWorld->GetID();
    return GetWorldInfo(ID).DstWindow;
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
