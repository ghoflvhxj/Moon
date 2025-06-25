#include "MyGame.h"
#include "MoonEngine.h"

#include "GraphicDevice.h"

#include "Renderer.h"

#include "Texture.h"
#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "PointLightComponent.h"
#include "SphereComponent.h"
#include "StaticMeshComponent.h"
#include "Camera.h"
#include "Player.h"
#include "DirectInput.h"
#include "DynamicMeshComponent.h"
#include "GameFramework/StaticMeshActor/StaticMeshActor.h"
#include "Core/ResourceManager.h"
#include "Material.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

using namespace DirectX;

MyGame::MyGame()
	: MainGame()
	, _pTerrainComponent{ nullptr }
	, _pPlayer{ nullptr }
{
	initialize();
	intializeImGui();
}

MyGame::~MyGame()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

const bool MyGame::initialize()
{
    MainGame::initialize();

	getMainCamera()->setLookMode(MCamera::LookMode::To);

	_pPlayer = CreateActor<Player>(this);

    LanternActor = CreateActor<MStaticMeshActor>(this);
    LanternActor->SetStaticMesh(TEXT("Lantern/Lantern.fbx"));;
    LanternActor->GetStaticMeshCompoent()->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
    LanternActor->GetStaticMeshCompoent()->setTranslation(0.f, 0.f, 0.f);
    LanternActor->GetStaticMeshCompoent()->setDrawingBoundingBox(true);

	//_pTerrainComponent = std::make_shared<TerrainComponent>(100, 100);
	//_pTerrainComponent->setTexture(EnumToIndex(TextureType::Diffuse), std::make_shared<TextureComponent>(TEXT("Resources/Texture/stone_01_albedo.jpg")));
	//_pTerrainComponent->setTexture(EnumToIndex(TextureType::Normal), std::make_shared<TextureComponent>(TEXT("Resources/Texture/Stone_01_normal.jpg")));
	//_pTerrainComponent->setTexture(EnumToIndex(TextureType::Specular), std::make_shared<TextureComponent>(TEXT("Resources/Texture/stone_01_Specular.jpg")));
	//_pTerrainComponent->getMaterial(0)

	//auto pComponent = getMainCamera()->getComponent(TEXT("RootComponent"));
	//if (nullptr == pComponent)
	//	return false;

	//pComponent->setTranslation(0.f, 0.f, -10.f);

	return true;
}

void MyGame::intializeImGui()
{
	// ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(getGraphicDevice()->getDevice(), getGraphicDevice()->getContext());

    io.Fonts->AddFontFromFileTTF("Resources/Fonts/NanumSquareRoundR.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
}

void MyGame::Tick(const Time deltaTime)
{
    if (InputManager::mouseDown(MOUSEBUTTON::LB))
    {
        Pick();
    }
}

void MyGame::render()
{
    std::shared_ptr<MLightComponent> DirectionalLight = std::static_pointer_cast<MLightComponent>(_pPlayer->getComponent(TEXT("DirectionalLight")));
    std::shared_ptr<MLightComponent> PointLight = std::static_pointer_cast<MLightComponent>(_pPlayer->getComponent(TEXT("PointLight")));
    std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = std::static_pointer_cast<DynamicMeshComponent>(_pPlayer->getComponent(TEXT("DynamicMesh")));
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hello, world!");                  
	ImGui::Text("Toatal primitive:%d", getRenderer()->TotalPrimitiveNum);
	ImGui::Text("show primitive:%d", getRenderer()->ShownPrimitiveNum);
	ImGui::Text("culled primitive:%d", getRenderer()->CulledPrimitiveNum);
	ImGui::Checkbox("Debug Collision", &getRenderer()->bDrawCollision);

	if (ImGui::CollapsingHeader("DirectionalLight") && DirectionalLight)
	{
		Vec3 rot = DirectionalLight->getRotation();
		ImGui::SliderAngle("rotX", &rot.x);
		ImGui::SliderAngle("rotY", &rot.y);
		ImGui::SliderAngle("rotZ", &rot.z);
        DirectionalLight->setRotation(rot);
	}

    // SceneComponent를 수정하게
    if (ImGui::CollapsingHeader("PointLight") && PointLight)
    {
        Vec3 Pos = PointLight->getTranslation();
        ImGui::SliderFloat("PosX", &Pos.x, -10, 10);
        ImGui::SliderFloat("PosY", &Pos.y, -10, 10);
        ImGui::SliderFloat("PosZ", &Pos.z, -10, 10);
        PointLight->setTranslation(Pos);
        Lantern->setTranslation(Pos.x, Pos.y - 1.f, Pos.z);
    }

	if (ImGui::CollapsingHeader("Actor"))
	{
		ImGui::SliderFloat("ForceY", &Force, 0.f, 10000.f);
		if (ImGui::Button("AddForce"))
		{
			Lantern->Temp(Force);
		}

		if (ImGui::Button("ResetPos"))
		{
			Lantern->setTranslation(0.f, 5.f, 0.f);
		}

		ImGui::Checkbox("DisableCollision", &bStaticCollision);
		if (Lantern)
		{
			Lantern->SetStaticCollision(bStaticCollision);
		}
	}

	if (ImGui::Button("SaveJson"))
	{
		_pPlayer->JsonTest();
	}
	if (ImGui::Button("SaveJsonPretty"))
	{
		_pPlayer->JsonTest(true);
	}

    if (DynamicMeshComp && ImGui::Button(DynamicMeshComp->IsAnimPlaying() ? "PauseAnim" : "PlayAnim"))
    {
        DynamicMeshComp->SetAnimPlaying(!DynamicMeshComp->IsAnimPlaying());
    }

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}