﻿#include "MyGame.h"
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
	getMainCamera()->setLookMode(MCamera::LookMode::To);

	_pPlayer = std::make_shared<Player>();
	addActor(_pPlayer);

	MyActor = std::make_shared<Actor>();
	//a = std::make_shared<SphereComponent>();
	//MyActor->addComponent(TEXT("Root"), a);
	Lantern = std::make_shared<StaticMeshComponent>(TEXT("Lantern/Lantern.fbx"), true, false);
	Lantern->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
	Lantern->setTranslation(0.f, 0.f, 0.f);
	Lantern->setDrawingBoundingBox(true);
	MyActor->addComponent(TEXT("test"), Lantern);
	//a->AddChildComponent(_pStaticMeshComponent);
	addActor(MyActor);

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
}

void MyGame::Tick(const Time deltaTime)
{
	controlCamera(deltaTime);

	//_pTerrainComponent->Update(deltaTime);
	//_pPlayer->rideTerrain(_pTerrainComponent);

	if (MyActor && bButtonPressed)
	{
		//_pStaticMeshComponent->Temp(Force);
		//a->AddForce(Vec3(0.f, Force, 0.f));
	}
}

void MyGame::render()
{
	//std::shared_ptr<LightComponent> p = std::static_pointer_cast<LightComponent>(_pPlayer->getComponent(TEXT("PointLight")));

	//Vec3 pos = p->getTranslation();
	//ImGui::SliderFloat("posX", &pos.x, -10.f, 10.f);
	//ImGui::SliderFloat("posY", &pos.y, -10.f, 10.f);
	//ImGui::SliderFloat("posZ", &pos.z, -10.f, 10.f);
	//p->setTranslation(pos);

    std::shared_ptr<MLightComponent> DirectionalLight = std::static_pointer_cast<MLightComponent>(_pPlayer->getComponent(TEXT("DirectionalLight")));
    std::shared_ptr<MLightComponent> PointLight = std::static_pointer_cast<MLightComponent>(_pPlayer->getComponent(TEXT("PointLight")));

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

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void MyGame::controlCamera(const Time deltaTime)
{
    //auto pComponent = getMainCamera()->getComponent(TEXT("RootComponent"));
    //if (nullptr == pComponent)
    //    return;

    //Vec3 trans = pComponent->getTranslation();
    //Vec3 look = pComponent->GetForward();
    //Vec3 right = pComponent->getRight();
    //float speed = _cameraSpeedScale * 1.f * deltaTime;

    //if (InputManager::keyPress(DIK_LSHIFT))
    //{
    //    speed *= 5.f;
    //}

    //if (InputManager::keyPress(DIK_W))
    //{
    //    trans.x += look.x * speed;
    //    trans.y += look.y * speed;
    //    trans.z += look.z * speed;
    //}
    //else if (InputManager::keyPress(DIK_S))
    //{
    //    trans.x -= look.x * speed;
    //    trans.y -= look.y * speed;
    //    trans.z -= look.z * speed;
    //}
    //else if (InputManager::keyPress(DIK_D))
    //{
    //    trans.x += right.x * speed;
    //    trans.y += right.y * speed;
    //    trans.z += right.z * speed;
    //}
    //else if (InputManager::keyPress(DIK_A))
    //{
    //    trans.x -= right.x * speed;
    //    trans.y -= right.y * speed;
    //    trans.z -= right.z * speed;
    //}

    //_cameraSpeedScale += static_cast<float>(InputManager::mouseMove(MOUSEAXIS::Z)) / 10.f;
    //_cameraSpeedScale = _cameraSpeedScale >= 1.f ? _cameraSpeedScale : 1.f;

    //pComponent->setTranslation(trans);

    //if (InputManager::mousePress(MOUSEBUTTON::RB))
    //{
    //    Vec3 rot = pComponent->getRotation();

    //    float mouseX = static_cast<float>(InputManager::mouseMove(MOUSEAXIS::X));
    //    float mouseY = static_cast<float>(InputManager::mouseMove(MOUSEAXIS::Y));

    //    rot.x = rot.x + (((rot.x + mouseY) - rot.x) * 0.005f);
    //    rot.y = rot.y + (((rot.y + mouseX) - rot.y) * 0.005f);
    //    pComponent->setRotation(rot);
    //}
}