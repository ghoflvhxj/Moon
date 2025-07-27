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
#include "Mesh/StaticMesh/StaticMesh.h"

#include "Core/Physics/Physics.h"

// 테스트용
#include "Core/Serialize/JsonSerializer.h"

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

bool MainGame::Raycast(const std::vector<FPrimitiveData>& InPrimitives, FHitData& OutHitData)
{
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(g_hWnd, &MousePos);

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

    OutHitData = {};
    OutHitData.Distance = FLT_MAX;

    uint32 DataNum = GetSize(InPrimitives);
    for (uint32 DataIndex = 0; DataIndex < DataNum; ++DataIndex)
    {
        auto& PrimitiveData = InPrimitives[DataIndex];
        std::shared_ptr<MPrimitiveComponent> PrimitiveComponent = PrimitiveData.PrimitiveComponent.lock();

        if (PrimitiveComponent == nullptr)
        {
            continue;
        }

        if (PrimitiveComponent->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal)
        {
            continue;
        }

        if (PrimitiveData.PrimitiveType != EPrimitiveType::Mesh)
        {
            continue;
        }

        XMMATRIX InverseWorldMat = XMLoadFloat4x4(&PrimitiveComponent->GetInverseWorldMatrix());
        XMVECTOR Start = XMVector3TransformCoord(NearWorldPos, InverseWorldMat);
        XMVECTOR End = XMVector3TransformCoord(FarWorldPos, InverseWorldMat);
        XMVECTOR Dir = XMVector3Normalize(End - Start);

        const auto& MeshData = PrimitiveData.MeshData.lock();
        const auto& Vertices = MeshData->Vertices;
        const auto& Indices = MeshData->Indices;

        auto Lambda = [](const Vec4& Pos)->XMVECTOR {
            Vec3 OutPos = { Pos.x, Pos.y, Pos.z };
            return XMLoadFloat3(&OutPos);
        };

        uint32 Loop = GetSize(Indices) / 3;
        for (uint32 i = 0; i < Loop; ++i)
        {
            float LocalDistance = 0.f;
            if (TriangleTests::Intersects(Start, Dir, Lambda(Vertices[Indices[i * 3 + 0]].Pos), Lambda(Vertices[Indices[i * 3 + 1]].Pos), Lambda(Vertices[Indices[i * 3 + 2]].Pos), LocalDistance))
            {
                XMVECTOR LocalHitPos = Start + (Dir * LocalDistance);
                XMVECTOR WorldHitPos = XMVector3TransformCoord(LocalHitPos, XMLoadFloat4x4(&PrimitiveComponent->getWorldMatrix()));

                float WorldDistance = XMVectorGetX(XMVector3Length(WorldHitPos - NearWorldPos));
                if (WorldDistance > OutHitData.Distance)
                {
                    continue;
                }

                OutHitData.HitComponent = PrimitiveData.PrimitiveComponent;
                OutHitData.Distance = WorldDistance;
                OutHitData.PrimitiveIndex = DataIndex;
                XMStoreFloat3(&OutHitData.HitPos, WorldHitPos);
            }
        }
    }

    return OutHitData.HitComponent.expired() == false;
}

void MainGame::ScreenToWorld(const Vec2& InPos, float Depth, Vec3& OutPos) const
{
    // 스크린 -> NDC
    UINT Width = g_pSetting->getResolutionWidth<UINT>();
    UINT Height = g_pSetting->getResolutionHeight<UINT>();
    
    Vec3 NDC = {
        InPos.x / (Width / 2.f) - 1.f,
        InPos.y / -(Height / 2.f) + 1.f,
        Depth
    };

    XMVECTOR ViewPos = XMVector3TransformCoord(XMLoadFloat3(&NDC), XMLoadFloat4x4(&getMainCamera()->getInverseProjectionMatrix()));
    XMVECTOR WorldPos = XMVector3TransformCoord(ViewPos, XMLoadFloat4x4(&getMainCamera()->getInvesrViewMatrix()));

    XMStoreFloat3(&OutPos, WorldPos);
}

void MainGame::WorldToScreen(const Vec3& InPos, Vec2& OutPos) const
{
    float Width = g_pSetting->getResolutionWidth<float>();
    float Height = g_pSetting->getResolutionHeight<float>();

    XMVECTOR ViewPos = XMVector3TransformCoord(XMLoadFloat3(&InPos), XMLoadFloat4x4(&getMainCamera()->getViewMatrix()));
    XMVECTOR ProjectPos = XMVector3TransformCoord(ViewPos, XMLoadFloat4x4(&getMainCamera()->getProjectionMatrix()));
    //XMVECTOR NDCPos = ProjectPos / XMVectorGetZ(ProjectPos); XMVector3TransformCoord가 z나누기 해줌

    OutPos = { (XMVectorGetX(ProjectPos) + 1.f) * (Width / 2.f), (-XMVectorGetY(ProjectPos) + 1.f) * (Height / 2.f)};
}

void MainGame::ProjectVec3(const Vec3& InBase, const Vec3& InTarget, Vec3& Out) const
{
    XMVECTOR Base = XMVector3Normalize(XMLoadFloat3(&InBase));
    XMVECTOR Target = XMLoadFloat3(&InTarget);

    float Dot = XMVectorGetX(XMVector3Dot(Base, Target));

    XMVECTOR Proj = Base * (Dot * XMVector3Length(Target));
    XMStoreFloat3(&Out, Proj);
}

void MainGame::ProjectVec2(const Vec2& InBase, const Vec2& InTarget, Vec2& Out) const
{
    XMVECTOR Base = XMVector2Normalize(XMLoadFloat2(&InBase));
    XMVECTOR Target = XMLoadFloat2(&InTarget);

    float Dot = XMVectorGetX(XMVector2Dot(Base, Target));

    XMVECTOR Proj = Base * (Dot * XMVector2Length(Target));
    XMStoreFloat2(&Out, Proj);
}

const Vec2 MainGame::GetMousePos()
{
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(g_hWnd, &MousePos);

    return { static_cast<float>(MousePos.x), static_cast<float>(MousePos.y) };
}

