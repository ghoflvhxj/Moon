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
#include "Mesh/StaticMesh/StaticMesh.h"
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
    LanternActor->GetStaticMeshCompoent()->SetPhysicsType(EPhysicsType::Dynamic);
    LanternActor->SetStaticMesh(TEXT("Lantern/Lantern.fbx"));;
    LanternActor->GetStaticMeshCompoent()->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
    LanternActor->GetStaticMeshCompoent()->SetDrawCollision(true);
    //LanternActor->GetStaticMeshCompoent()->setTranslation(5.f, 0.f, 0.f);
    //LanternActor->GetStaticMeshCompoent()->setDrawingBoundingBox(true);

    ClothActor = CreateActor<MStaticMeshActor>(this);
    ClothActor->GetStaticMeshCompoent()->SetPhysics(false);
    ClothActor->SetStaticMesh(TEXT("Untitled.fbx"));
    ClothActor->GetStaticMeshCompoent()->setTranslation(0.f, 6.f, 0.f);

    ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, std::make_shared<MTexture>(TEXT("./Resources/Texture/stone_01_albedo.jpg")));
    ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Normal, std::make_shared<MTexture>(TEXT("./Resources/Texture/Stone_01_normal.jpg")));

    //std::shared_ptr<MTexture> Texture = nullptr;
    //if (g_ResourceManager->Load<MTexture>(TEXT("Resources/Texture/Player.jpeg"), Texture))
    //{
    //    ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, Texture);
    //    ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setCullMode(Graphic::CullMode::None);
    //    //ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setFillMode(Graphic::FillMode::WireFrame);
    //}

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

void MyGame::PostUpdate(const Time deltaTime)
{

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
        LanternActor->GetStaticMeshCompoent()->setTranslation(Pos.x, Pos.y - 1.f, Pos.z);
    }

	if (ImGui::CollapsingHeader("Actor") && LanternActor)
	{
        Vec3 Pos = LanternActor->GetStaticMeshCompoent()->getTranslation();
        Vec3 NewPos = Pos;
        ImGui::SliderFloat("PosX", &NewPos.x, -30, 30);
        ImGui::SliderFloat("PosY", &NewPos.y, -30, 30);
        ImGui::SliderFloat("PosZ", &NewPos.z, -30, 30);

        auto IsNotEqual = [](float lhs, float rhs)->bool {
            return std::fabsf(lhs - rhs) > 0.00001;
        };

        if (IsNotEqual(Pos.x, NewPos.x) || IsNotEqual(Pos.y, NewPos.y) || IsNotEqual(Pos.z, NewPos.z))
        {
            LanternActor->GetStaticMeshCompoent()->setTranslation(NewPos.x, NewPos.y, NewPos.z);
        }

		ImGui::SliderFloat("ForceY", &Force, 0.f, 10000.f);
		if (ImGui::Button("AddForce"))
		{
            LanternActor->GetStaticMeshCompoent()->Temp(Force);
		}

        if (ImGui::Button("ResetVelocity"))
        {
            LanternActor->GetStaticMeshCompoent()->SetVelocity(0.f, 0.f, 0.f);
            LanternActor->GetStaticMeshCompoent()->SetAngularVelocity(0.f, 0.f, 0.f);
        }

		if (ImGui::Button("ResetPos"))
		{
            LanternActor->GetStaticMeshCompoent()->setTranslation(0.f, 5.f, 0.f);
		}

        if (ImGui::Checkbox("Physics Simulation", &bStaticCollision))
        {
            LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(bStaticCollision);
        }
	}

    if (ImGui::CollapsingHeader("JsonTest"))
    {
        ImGui::Indent(20);
        if (ImGui::Button("SaveJson"))
        {
            _pPlayer->JsonSaveTest();
        }
        if (ImGui::Button("SaveJsonPretty"))
        {
            _pPlayer->JsonSaveTest(true);
        }
        if (ImGui::Button("LoadJson"))
        {
            _pPlayer->JsonLoadTest();
        }
        ImGui::Indent(-20);
    }

    if (DynamicMeshComp && ImGui::Button(DynamicMeshComp->IsAnimPlaying() ? "PauseAnim" : "PlayAnim"))
    {
        //std::random_device rd;
        //std::mt19937 gen(rd());
        //std::uniform_int_distribution<int> AnimClipIndex(0, DynamicMeshComp->GetAnimClipNum());
        //DynamicMeshComp->SetAnimClip(AnimClipIndex(gen));
        DynamicMeshComp->SetAnimClip(0);
        DynamicMeshComp->SetAnimPlaying(!DynamicMeshComp->IsAnimPlaying());
    }

    if(ClothActor && ImGui::Button("Cloth"))
    {
        ClothActor->GetStaticMeshCompoent()->Clothing();
    }

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}