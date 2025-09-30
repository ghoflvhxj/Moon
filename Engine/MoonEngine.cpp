#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Window.h"
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

std::unique_ptr<MainGameSetting> g_pSetting	= std::make_unique<MainGameSetting>();
std::unique_ptr<MResourceManager> g_ResourceManager	= std::make_unique<MResourceManager>();

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
    g_Engine->Initialize();

    g_World = std::make_unique<MWorld>();
    g_World->Initialize();
    g_World->GetGameStartedDelegate().Add([&]() {
        if (GetPhysics())
        {
            GetPhysics()->StartSimulate();
        }
    });

	return true;
}

ENGINE_DLL void EngineLoop()
{
    if (EngineUpdate())
    {
        g_World->Loop();
        GetEngine()->Update();
        EngineRender();

        float Current = GetEngine()->TimerManager.GetCurrent();
        float ElapsedTimeForUpdate = Current - GetEngine()->PrevProcessTime;

        //std::cout << ElapsedTimeForUpdate << std::endl;

        GetEngine()->FrameManager.SetDeltaTime(ElapsedTimeForUpdate);
        GetEngine()->PrevProcessTime = GetEngine()->TimerManager.GetCurrent();
    }


}

bool EngineUpdate()
{
    MTimerManager& TimerManager = GetEngine()->TimerManager;
    MFrameManager& FrameManager = GetEngine()->FrameManager;

    TimerManager.Tick();

    float Current = TimerManager.GetCurrent();
    float ElapsedTimeForUpdate = Current - GetEngine()->PrevProcessTime;

    // 월드 업데이트&렌더의 완료에 걸리는 시간이 프레임 당 시간보다 적으면
    // 일찍 작업이 끝난거니 업데이트는 기다림
    return ElapsedTimeForUpdate >= FrameManager.GetTimePerFrame();
}

void EngineRender()
{
    if (g_pGraphicDevice)
    {
        g_pGraphicDevice->Begin();
        OnRenderStartedDelegate.Broadcast();

        GetEngine()->Render();

        OnRenderFinishedDelegate.Broadcast();
        g_pGraphicDevice->End();
    }
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

	g_ResourceManager->Release();

    GetEngine()->Release();
    getGraphicDevice()->Release();

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

void SetModule(std::unique_ptr<MModule>&& InModule)
{
    //g_Module = std::move(InModule);
    //g_Module->Initialize();
}

void RegisterComponent(std::shared_ptr<Component> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = InComponent->CastTo<MPrimitiveComponent>())
    {
        getRenderer()->AddPrimitiveComponent(std::static_pointer_cast<MPrimitiveComponent>(InComponent));
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

void MEngine::Initialize()
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

void MEngine::Update()
{
    for (auto& Module : Modules)
    {
        Module->Update();
    }

    GetOnUpdated().Broadcast();
}

void MEngine::Render()
{
    for (auto& Module : Modules)
    {
        Module->Render();
    }
}

void MEngine::Release()
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
