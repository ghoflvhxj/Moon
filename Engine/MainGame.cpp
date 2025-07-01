#include "Include.h"
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

#include "MainGameSetting.h"
#include "Camera.h"
#include "MeshComponent.h"
#include "StaticMeshComponent.h"
#include "Core/StaticMesh/StaticMesh.h"

#include "Core/Physics/Physics.h"

using namespace DirectX;

MainGame::MainGame()
	: _deltaTime{ 0.f }
	, _pTimerManager{ nullptr }
	, _pFrameManager{ nullptr }
	
	//, _pMainGameSetting{ std::make_shared<MainGameSetting>() }
	, _pMainCamera(std::make_shared<MCamera>(g_pSetting->getFov()))
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

	if (_pMainCamera)
	{
		_pMainCamera->update(_deltaTime);
	}

	Tick(_deltaTime);
	Update(_deltaTime);
    
    if (g_pPhysics)
    {
        g_pPhysics->Update(_deltaTime);
    }

    PostUpdate(_deltaTime);

	if (g_pRenderer && g_pGraphicDevice)
	{
        g_pGraphicDevice->Begin();
		g_pRenderer->Render();
        g_pGraphicDevice->End();
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
	_pTimerManager = std::make_shared<MTimerManager>();
	_pFrameManager = std::make_shared<FrameManager>(_pTimerManager);

	return true;
}

const std::shared_ptr<MTimerManager> MainGame::getTimerManager() const
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

void MainGame::SetMainCamera(std::shared_ptr<MCamera> pCamera)
{
	_pMainCamera = pCamera;
}

std::shared_ptr<MCamera> MainGame::getMainCamera() const
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

void MainGame::Pick()
{
    //Vec2 GetMousePosition()
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(g_hWnd, &MousePos);
    std::cout << "Screen: " << MousePos.x << ", " << MousePos.y << std::endl;

    // 스크린 -> NDC
    UINT Width = g_pSetting->getResolutionWidth<UINT>();
    UINT Height = g_pSetting->getResolutionHeight<UINT>();
    Vec3 NearNdc, FarNdc;
    NearNdc.x = FarNdc.x = MousePos.x / (Width / 2.f) - 1.f;
    NearNdc.y = FarNdc.y = MousePos.y / -(Height / 2.f) + 1.f;
    NearNdc.z = 0.f;
    FarNdc.z = 1.f;

    // NearNDC -> NDC -> 뷰 -> 월드
    XMVECTOR NearViewPos = XMVector3TransformCoord(XMLoadFloat3(&NearNdc), XMLoadFloat4x4(&getMainCamera()->getInverseProjectionMatrix()));
    XMVECTOR NearWorldPos = XMVector3TransformCoord(NearViewPos, XMLoadFloat4x4(&getMainCamera()->getInvesrViewMatrix()));

    XMVECTOR FarViewPos = XMVector3TransformCoord(XMLoadFloat3(&FarNdc), XMLoadFloat4x4(&getMainCamera()->getInverseProjectionMatrix()));
    XMVECTOR FarWorldPos = XMVector3TransformCoord(FarViewPos, XMLoadFloat4x4(&getMainCamera()->getInvesrViewMatrix()));

    Vec3 RayDirection; 
    Vec3 RayStart;
    XMStoreFloat3(&RayDirection, XMVector3Normalize(FarWorldPos - NearWorldPos));
    XMStoreFloat3(&RayStart, NearWorldPos);

    const auto& MeshPrimitives = g_pRenderer->GetRenderablePrimitiveData();
    for (auto& PrimitiveData : MeshPrimitives)
    {
        if (PrimitiveData.PrimitiveType != EPrimitiveType::Mesh)
        {
            continue;
        }

        XMMATRIX InverseWorldMat = XMLoadFloat4x4(&PrimitiveData.PrimitiveComponent.lock()->GetInverseWorldMatrix());
        XMVECTOR Start = XMVector3TransformCoord(NearWorldPos, InverseWorldMat);
        XMVECTOR End = XMVector3TransformCoord(FarWorldPos, InverseWorldMat);
        XMVECTOR Dir = XMVector3Normalize(End - Start);

        const auto& Vertices = PrimitiveData.MeshData.lock()->Vertices;
        const auto& Indices = PrimitiveData.MeshData.lock()->Indices;
        uint32 Loop = GetSize(Indices) / 3;
        for (uint32 i = 0; i < Loop; ++i)
        {
            float Distance = 0.f;
            if (TriangleTests::Intersects(Start, Dir, XMLoadFloat4(&Vertices[Indices[i * 3 + 0]].Pos), XMLoadFloat4(&Vertices[Indices[i * 3 + 1]].Pos), XMLoadFloat4(&Vertices[Indices[i * 3 + 2]].Pos), Distance))
            {
                std::cout << "Intersect!" << std::endl;
                return;
            }

            //XMVECTOR Plane = XMPlaneFromPoints();
            //XMVECTOR Result = XMPlaneIntersectLine(Plane, Start, End);

            //if (XMVector3IsNaN(Result) == false)
            //{
            //    XMVECTOR DirToResult = Result - Start;
            //    if (XMVectorGetX(XMVector3Dot(DirToResult, Dir)) > 0.f)
            //    {
            //        std::cout << "Intersect!" << std::endl;
            //        return;
            //    }
            //}
        }
    }
}
