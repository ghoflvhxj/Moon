#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Window.h"
#include "DirectInput.h"
#include "GraphicDevice.h"
#include "Renderer.h"
#include "MainGame.h"
#include "Core/Physics/PhysX/MPhysX.h"
#include "Core/Physics/Jolt/Jolt.h"

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
std::shared_ptr<Window> g_pMainWindow				= nullptr;
std::unique_ptr<DirectInput> g_pDirectInput			= nullptr;
std::unique_ptr<GraphicDevice> g_pGraphicDevice		= nullptr;
std::unique_ptr<MShaderManager> ShaderManager		= nullptr;
std::unique_ptr<Renderer> g_pRenderer				= nullptr;
std::shared_ptr<MainGame> g_pMainGame				= nullptr;
std::unique_ptr<MPhysicsEngine> g_pPhysics			= nullptr;
ENGINE_DLL std::unique_ptr<MResourceManager> g_ResourceManager	= nullptr;
FDelegate<void> PostLoopDeleagate;
FDelegate<void> OnLevelChangedDelegate;
		

const bool EngineInit(const HINSTANCE hInstance, std::shared_ptr<Window> pWindow)
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

	return true;
}

const bool EngineLoop()
{
    // 피직스, 오디오 엔진 등이 Game의 Loop가 아니라 이곳에서 동작하도록 구조를 개선해야 함.
     
    //if (g_pPhysics)
    //{
    //    g_pPhysics->Update(_deltaTime);
    //}

	return g_pMainGame->Loop();
}

ENGINE_DLL void EnginePostLoop()
{
    PostLoopDeleagate.Broadcast();
    PostLoopDeleagate.Clear();
}

const bool EngineRelease()
{
    LOG(std::wstring(TEXT("Game Reset Start")));
	g_pMainGame.reset();
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

std::unique_ptr<Renderer>& getRenderer()
{
	return g_pRenderer;
}

std::shared_ptr<MainGame>& getMainGame()
{
	return g_pMainGame;
}

std::unique_ptr<MainGameSetting>& getSetting()
{
	return g_pSetting;
}

ENGINE_DLL std::unique_ptr<MPhysicsEngine>& GetPhysics()
{
    return g_pPhysics;
}

const bool setGame(std::unique_ptr<MainGame>&& pGame)
{
	g_pMainGame = std::move(pGame);
    g_pMainGame->initialize();

    g_pMainGame->GetGameStartedDelegate().Add([&]() {
        if (GetPhysics())
        {
            GetPhysics()->StartSimulate();
        }
    });

	return true;
}

void RegisterComponent(std::shared_ptr<Component> InComponent)
{
    if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = InComponent->CastTo<MPrimitiveComponent>())
    {
        getRenderer()->AddPrimitive(std::static_pointer_cast<MPrimitiveComponent>(InComponent));
    }

    if (std::shared_ptr<MMeshComponent> MeshComp = InComponent->CastTo<MMeshComponent>())
    {
        //std::weak_ptr<MMeshComponent> WeakMeshComp = MeshComp;
        //MeshComp->GetBeganPlay().Add([WeakMeshComp]() {
        //    GetPhysics()->Temp(WeakMeshComp.lock());
        //});
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
