#include "stdafx.h"
#include "MyGame.h"

#include "MoonEngine.h"

#include "GraphicDevice.h"

#include "Renderer.h"

#include "TextureComponent.h"
#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "PointLightComponent.h"
#include "Camera.h"
#include "Player.h"

#include "Misc/imgui.h"
#include "Misc/backends/imgui_impl_win32.h"
#include "Misc/backends/imgui_impl_dx11.h"

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
	getMainCamera()->setLookMode(Camera::LookMode::To);

	_pPlayer = std::make_shared<Player>();
	addActor(_pPlayer);

	//_pTerrainComponent = std::make_shared<TerrainComponent>(100, 100);
	//_pTerrainComponent->setTexture(enumToIndex(TextureType::Diffuse), std::make_shared<TextureComponent>(TEXT("Resources/Texture/stone_01_albedo.jpg")));
	//_pTerrainComponent->setTexture(enumToIndex(TextureType::Normal), std::make_shared<TextureComponent>(TEXT("Resources/Texture/Stone_01_normal.jpg")));
	//_pTerrainComponent->setTexture(enumToIndex(TextureType::Specular), std::make_shared<TextureComponent>(TEXT("Resources/Texture/stone_01_Specular.jpg")));
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
	// ÇÃ·§Æû/·»´õ·¯¸¦ ¹ÙÀÎµù
	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(getGraphicDevice()->getDevice(), getGraphicDevice()->getContext());
}

void MyGame::Tick(const Time deltaTime)
{
	controlCamera(deltaTime);

	//_pTerrainComponent->Update(deltaTime);
	//_pPlayer->rideTerrain(_pTerrainComponent);
}

void MyGame::render()
{
	//std::shared_ptr<PointLightComponent> p	= std::static_pointer_cast<PointLightComponent>(_pPlayer->getComponent(TEXT("DirectionalLight")));
	//std::shared_ptr<SceneComponent>	p2		= _pPlayer->getComponent(TEXT("test2"));
	////Vec3 color = p->getColor();
	////float range = p->getRange();
	////Vec3 pos = p->getTranslation();
	////Vec3 rot = p->getRotation();
	////float intensitiy = p->getIntensity();

	//Vec3 color = VEC3ZERO;
	//float range = 0.f;
	//float intensity = 0.f;

	//Vec3 pos = p2->getScale();
	//Vec3 rot = p2->getScale();
	//Vec3 scale = p2->getScale();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
	ImGui::Text("Toatal primitive:%d", getRenderer()->totalPrimitiveCount);
	ImGui::Text("show primitive:%d", getRenderer()->showPrimitiveCount);
	ImGui::Text("culled primitive:%d", getRenderer()->culledPrimitiveCount);
	////ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	////ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::SliderFloat("posX", &pos.x, -10.f, 10.f);
	//ImGui::SliderFloat("posY", &pos.y, -10.f, 10.f);
	//ImGui::SliderFloat("posZ", &pos.z, -10.f, 10.f);
	//ImGui::SliderFloat("scaleX", &scale.x, 0.f, 1.f);
	//ImGui::SliderFloat("scaleY", &scale.y, 0.f, 1.f);
	//ImGui::SliderFloat("scaleZ", &scale.z, 0.f, 1.f);
	//ImGui::SliderAngle("rotationX", &rot.x);
	//ImGui::SliderAngle("rotationY", &rot.y);
	//ImGui::SliderAngle("rotationZ", &rot.z);
	//ImGui::ColorEdit3("clear color", (float *)&color);
	//ImGui::SliderFloat("range", &range, 0.f, 100.f);
	//ImGui::SliderFloat("intensity", &intensity, 0.f, 10.f);
	////p->setTranslation(pos);
	////p->setRotation(rot);
	////p->setColor(color);
	////p->setRange(range);
	////p->setIntensity(intensitiy);

	//p2->setTranslation(pos);
	//p2->setScale(scale);
	//p2->setRotation(rot);

	//bool show = true;
	//ImGui::Checkbox("Dynamicmesh Show", &show);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void MyGame::controlCamera(const Time deltaTime)
{
	auto pComponent = getMainCamera()->getComponent(TEXT("RootComponent"));
	if (nullptr == pComponent)
		return;

	Vec3 trans = pComponent->getTranslation();
	Vec3 look = pComponent->getLook();
	Vec3 right = pComponent->getRight();
	float speed = _cameraSpeedScale * 1.f * deltaTime;

	if (keyPress(DIK_LSHIFT))
	{
		speed *= 5.f;
	}

	if (keyPress(DIK_W))
	{
		trans.x += look.x * speed;
		trans.y += look.y * speed;
		trans.z += look.z * speed;
	}
	else if (keyPress(DIK_S))
	{
		trans.x -= look.x * speed;
		trans.y -= look.y * speed;
		trans.z -= look.z * speed;
	}
	else if (keyPress(DIK_D))
	{
		trans.x += right.x * speed;
		trans.y += right.y * speed;
			trans.z += right.z * speed;
	}
	else if (keyPress(DIK_A))
	{
		trans.x -= right.x * speed;
		trans.y -= right.y * speed;
		trans.z -= right.z * speed;
	}

	_cameraSpeedScale += static_cast<float>(mouseMove(MOUSEAXIS::Z)) / 10.f;
	_cameraSpeedScale = _cameraSpeedScale >= 1.f ? _cameraSpeedScale : 1.f;

	pComponent->setTranslation(trans);

	if (mousePress(MOUSEBUTTON::RB))
	{
		Vec3 rot = pComponent->getRotation();

		float mouseX = static_cast<float>(mouseMove(MOUSEAXIS::X));
		float mouseY = static_cast<float>(mouseMove(MOUSEAXIS::Y));

		rot.x = rot.x + (((rot.x + mouseY) - rot.x) * 0.005f);
		rot.y = rot.y + (((rot.y + mouseX) - rot.y) * 0.005f);
		pComponent->setRotation(rot);
	}
}