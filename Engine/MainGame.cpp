#include "stdafx.h"
#include "MainGame.h"

// System
#include "Thread.h"
#include "Window.h"
#include "TimerManager.h"
#include "FrameManager.h"

// Input
#include "DirectInput.h"

// Graphics
#include "GraphicDevice.h"

// Renderer
#include "Renderer.h"

// Framework
#include "MainGameSetting.h"
#include "Camera.h"
#include "MeshComponent.h"

MainGame::MainGame()
	: _deltaTime{ 0.f }
	, _pTimerManager{ nullptr }
	, _pFrameManager{ nullptr }
	
	//, _pMainGameSetting{ std::make_shared<MainGameSetting>() }
	, _pMainCamera(std::make_shared<Camera>(g_pSetting->getFov()))
{
}

const bool MainGame::Loop()
{
	_pTimerManager->Tick();
	_pFrameManager->Tick();
	_deltaTime += _pTimerManager->GetDeltaTime();

	if (_pFrameManager->IsLock())
		return false;

	g_pDirectInput->update();

	if (nullptr != _pMainCamera)
	{
		_pMainCamera->update(_deltaTime);
	}

	Tick(_deltaTime);
	Update(_deltaTime);

	if (nullptr != g_pRenderer)
	{
		g_pRenderer->render();
	}

	_deltaTime = 0.f;

	return true;
}

void MainGame::Tick(const Time deltaTime)
{
}

void MainGame::Update(const Time deltaTime)
{
	for (auto pActor : _actorList)
	{
		pActor->update(deltaTime);
	}
}

void MainGame::render()
{
}

void MainGame::addActor(std::shared_ptr<Actor> pActor)
{
	_actorList.push_back(pActor);
}

const Time MainGame::getDeltaTime() const
{
	return _deltaTime;
}

const bool MainGame::initialize()
{
	_pTimerManager = std::make_shared<TimerManager>(shared_from_this());
	_pFrameManager = std::make_shared<FrameManager>(shared_from_this());

	return true;
}

const std::shared_ptr<TimerManager> MainGame::getTimerManager() const
{
	return (_pTimerManager) ? _pTimerManager : nullptr;
}

const std::shared_ptr<FrameManager> MainGame::getFrameManager() const
{
	return _pFrameManager;
}

const Frame MainGame::getFrame() const
{
	return getFrameManager()->GetFrame();
}

//const std::shared_ptr<MainGameSetting> MainGame::getSetting()
//{
//	return _pMainGameSetting;
//}

void MainGame::SetMainCamera(std::shared_ptr<Camera> pCamera)
{
	_pMainCamera = pCamera;
}

std::shared_ptr<Camera> MainGame::getMainCamera() const
{
	return _pMainCamera;
}

const Mat4& MainGame::getMainCameraViewMatrix() const
{
	return (nullptr == _pMainCamera) ? IDENTITYMATRIX : _pMainCamera->getViewMatrix();
}

const Mat4& MainGame::getMainCameraProjectioinMatrix() const
{
	return (nullptr == _pMainCamera) ? IDENTITYMATRIX : _pMainCamera->getProjectionMatrix();
}

const Mat4& MainGame::getMainCameraOrthographicProjectionMatrix() const
{
	return (nullptr == _pMainCamera) ? IDENTITYMATRIX : _pMainCamera->getOrthographicProjectionMatrix();
}
