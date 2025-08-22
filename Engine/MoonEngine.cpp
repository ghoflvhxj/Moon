#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Window.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"
#include "World.h"
#include "Core/Physics/PhysX/MPhysX.h"
#include "Core/Physics/Jolt/Jolt.h"
#include "Core/Module/Module.h"

#include "ShaderManager.h"
#include "ShaderLoader.h"

#include "Core/ResourceManager.h"
#include "Core/ResourceLoader.h"

#include "Component.h"
#include "PrimitiveComponent.h"
#include "MeshComponent.h"

HINSTANCE g_hInstance;
HWND g_hWnd;

std::unique_ptr<MainGameSetting> g_pSetting			= std::make_unique<MainGameSetting>();
std::shared_ptr<MWindow> g_pMainWindow				= nullptr;
std::unique_ptr<DirectInput> g_pDirectInput			= nullptr;
std::unique_ptr<GraphicDevice> g_pGraphicDevice		= nullptr;
std::unique_ptr<MShaderManager> ShaderManager		= nullptr;
std::unique_ptr<Renderer> g_pRenderer				= nullptr;
std::shared_ptr<MWorld> g_World				        = nullptr;
std::unique_ptr<MPhysicsEngine> g_pPhysics			= nullptr;
std::unique_ptr<MModule> g_Module = nullptr;
ENGINE_DLL std::unique_ptr<MResourceManager> g_ResourceManager	= nullptr;
FDelegate<void> PostLoopDeleagate;
FDelegate<void> OnLevelChangedDelegate;
FDelegate<void> OnRenderFinishedDelegate;
FDelegate<void> OnRenderStartedDelegate;
		

const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<MWindow> pWindow)
{
	g_hInstance = hInstance;
	g_hWnd = pWindow->getHandle();

	g_pMainWindow		= pWindow;

    g_pDirectInput = std::make_unique<DirectInput>();
    g_pGraphicDevice = std::make_unique<GraphicDevice>();

    ShaderManager = std::make_unique<MShaderManager>();
    ShaderLoader shaderLoader;
    shaderLoader.loadShaderFiles(ShaderManager);

    g_ResourceManager = std::make_unique<MResourceManager>();

    g_pGraphicDevice->BuildInputLayout();

    g_pPhysics = std::make_unique<MJoltPhysics>();
    g_pRenderer = std::make_unique<Renderer>();

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

void EngineLoop()
{
    if (g_World->Loop())
    {

    }

    if (g_Module)
    {
        g_Module->Update();
    }

    if (g_pGraphicDevice)
    {
        g_pGraphicDevice->Begin();
        if (g_pRenderer)
        {
            OnRenderStartedDelegate.Broadcast();
            g_pRenderer->Render();
            OnRenderFinishedDelegate.Broadcast();
        }
        g_pGraphicDevice->End();
    }

    if (g_pPhysics)
    {
        g_pPhysics->Update(g_World->getDeltaTime());
    }
}

ENGINE_DLL void EnginePostLoop()
{
    PostLoopDeleagate.Broadcast();
    PostLoopDeleagate.Clear();
}

const bool EngineRelease()
{
    LOG(std::wstring(TEXT("Game Reset Start")));
	g_World.reset();
    LOG(std::wstring(TEXT("Game Reset Finish")));

	ShaderManager->Release();
	g_pRenderer->Release();
	g_ResourceManager->Release();
    g_pPhysics->Release();
	g_pGraphicDevice->Release();

    ReleaseReflection();

	return true;
}

std::unique_ptr<GraphicDevice>& getGraphicDevice()
{
	return g_pGraphicDevice;
}

ENGINE_DLL std::shared_ptr<MWindow>& GetMainWindow()
{
    return g_pMainWindow;
}

std::unique_ptr<Renderer>& getRenderer()
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

ENGINE_DLL std::unique_ptr<MPhysicsEngine>& GetPhysics()
{
    return g_pPhysics;
}

void SetModule(std::unique_ptr<MModule>&& InModule)
{
    g_Module = std::move(InModule);
    g_Module->Initialize();

	//g_pMainGame = std::move(pGame);
 //   g_pMainGame->initialize();

 //   g_pMainGame->GetGameStartedDelegate().Add([&]() {
 //       if (GetPhysics())
 //       {
 //           GetPhysics()->StartSimulate();
 //       }
 //   });

	//return true;
}

void RegisterComponent(std::shared_ptr<Component> InComponent)
{
    if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = InComponent->CastTo<MPrimitiveComponent>())
    {
        getRenderer()->AddPrimitive(std::static_pointer_cast<MPrimitiveComponent>(InComponent));
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
