#include "World.h"

#include "MoonEngine.h"

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

#include "Module/Physics/Physics.h"

// 테스트용
#include "Core/Serialize/JsonSerializer.h"

using namespace DirectX;

MWorld::MWorld()
	: _deltaTime{ 0.f }
{
    _pFrameManager = std::make_shared<MFrameManager>();
    _pTimerManager = std::make_shared<MTimerManager>();

    static uint32 StaticID = 0;
    ID = StaticID++;
}

MWorld::~MWorld()
{
    std::wstring Msg = TEXT("World Delete");
    LOG(Msg);
}

bool MWorld::Update()
{
    _deltaTime = GetEngine()->FrameManager.GetTimePerFrame();

    if (_pMainCamera)
    {
        _pMainCamera->update(_deltaTime);
    }

    if (bHasBegan)
    {
        for (auto& [Name, Actor] : Actors)
        {
            if (Actor->IsUpdatable() == false)
            {
                continue;
            }

            Actor->update(_deltaTime);
        }
    }

    // 시작 여부와 상관없는 업데이트
    for (auto Iter = AlwaysUpdatableComponents.begin(); Iter != AlwaysUpdatableComponents.end();)
    {
        if (Iter->expired())
        {
            Iter = AlwaysUpdatableComponents.erase(Iter);
            continue;
        }
        else
        {
            auto Comp = Iter->lock();
            if (Comp->isUpdateable())
            {
                Comp->Update(_deltaTime);
                Comp->OnUpdated();
            }
            ++Iter;
        }
    }
    
    // 액터 삭제
    GetPostLoopDelegate().Add([&]() {
        for (auto& Iter = DestroyQueue.begin(); Iter != DestroyQueue.end(); ++Iter)
        {
            std::string TargetActorName;
            for (auto [Name, Actor] : Actors)
            {
                if (Actor.get() == *Iter)
                {
                    TargetActorName = Name;
                    break;
                }
            }

            if (TargetActorName.empty() == false)
            {
                Actors[TargetActorName]->Destroy();
                Actors.erase(TargetActorName);
            }
        }

        DestroyQueue.clear();
    });

    TotalTime += _deltaTime;

	return true;
}

void MWorld::AddAlwaysUpdatableComponent(const std::shared_ptr<MSceneComponent>& InComp)
{
    assert(InComp);

    AlwaysUpdatableComponents.push_back(InComp);
}

void MWorld::PlayGame()
{
    if (bHasBegan)
    {
        return;
    }

    if (bHasBegan == false)
    {
        bHasBegan = true;

        for (auto& [Name, Actor] : Actors)
        {
            Actor->BeginPlay();
        }
    }

    std::wstring Message = TEXT("PlayGame");
    LOG(Message);

    GetGameStartedDelegate().Broadcast();
}

void MWorld::FinishGame()
{
    for (auto& [Name, Actor] : Actors)
    {
        Actor->Destroy();
    }
}

void MWorld::render()
{
}

void MWorld::addActor(std::shared_ptr<MActor> InActor)
{
    const FTypeDesc* TypeDesc = InActor->GetTypeDesc();
    std::string Name = "";
    while (Actors.find(Name) != Actors.end() || Name.empty())
    {
        uint32 ActorIndex = Indexer[TypeDesc]++;
        Name = TypeDesc->Name + "_" + std::to_string(ActorIndex);
    }

	Actors.emplace(Name, InActor);

    InActor->SetOwner(GetShared());
    InActor->RegistComponents();
    InActor->update(0.f);

    if (bHasBegan)
    {
        InActor->BeginPlay();
    }
}

Time MWorld::getDeltaTime() const
{
	return _deltaTime;
}

Time MWorld::GetTotalTime() const
{
    return TotalTime;
}

void MWorld::OnLoaded()
{
    LOG(std::wstring(TEXT("World Loaded!!!")));

    for (auto& [Name, Actor] : Actors)
    {
        Actor->SetOwner(GetShared());
        Actor->RegistComponents();
        Actor->PostConstruct();
        Actor->update(0.f);
    }
}

std::shared_ptr<MObject> MWorld::Duplicate()
{
    std::shared_ptr<MObject> DuplicatedObject = Super::Duplicate();

    if (DuplicatedObject == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<MWorld> DuplicatedWorld = DuplicatedObject->CastToShared<MWorld>();
    if (DuplicatedWorld == nullptr)
    {
        assert(false);
    }

    DuplicatedWorld->Actors.clear();

    return DuplicatedObject;
}

void MWorld::DuplicateActors(const std::shared_ptr<MWorld>& InSrcWorld)
{
    Actors.clear();

    for (auto& [Name, Actor] : InSrcWorld->Actors)
    {
        if (std::shared_ptr<MObject> NewObject = DuplicateObject(Actor))
        {
            std::shared_ptr<MActor> DuplicatedActor = NewObject->CastToShared<MActor>();
            addActor(DuplicatedActor);
        }
    }
}

void MWorld::RemoveActor(MActor* InActor)
{
    DestroyQueue.push_back(InActor);
}

const bool MWorld::Initialize()
{
    if (_pMainCamera == nullptr)
    {
        _pMainCamera = CreateActor<MCamera>(GetShared());
        _pMainCamera->setFov(g_pSetting->getFov());
        _pMainCamera->setLookMode(MCamera::LookMode::To);
    }

	return true;
}

std::shared_ptr<MTimerManager>& MWorld::getTimerManager() const
{
    return _pTimerManager;
}

std::shared_ptr<MFrameManager>& MWorld::getFrameManager() const
{
	return _pFrameManager;
}

const Frame MWorld::getFrame() const
{
	return getFrameManager()->GetFrame();
}

//const std::shared_ptr<MainGameSetting> MainGame::getSetting()
//{
//	return _pMainGameSetting;
//}

void MWorld::SetMainCamera(std::shared_ptr<MCamera> pCamera)
{
	_pMainCamera = pCamera;
}

std::shared_ptr<MCamera> MWorld::getMainCamera() const
{
	return _pMainCamera;
}

const Mat4& MWorld::getMainCameraViewMatrix() const
{
	return (nullptr == _pMainCamera) ? IDENTITYMATRIX : _pMainCamera->getViewMatrix();
}

const Mat4& MWorld::getMainCameraProjectioinMatrix() const
{
	return (nullptr == _pMainCamera) ? IDENTITYMATRIX : _pMainCamera->getProjectionMatrix();
}

const Mat4& MWorld::getMainCameraOrthographicProjectionMatrix() const
{
	return (nullptr == _pMainCamera) ? IDENTITYMATRIX : _pMainCamera->getOrthographicProjectionMatrix();
}

bool MWorld::Raycast(FHitData& OutHitData, ECollisionType InCollisionType)
{
    //for (auto& [Name, Actor] : Actors)
    //{
    //    Actor->
    //}

    /*
    const auto& Scene = getRenderer()->GetScene(GetID());
    return Raycast(Scene->GetRenderablePrimitiveData(), OutHitData);
    */

    return false;
}

bool MWorld::Raycast(const std::vector<FPrimitiveData>& InPrimitives, FHitData& OutHitData, uint8 InPrimitiveType)
{
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(g_hWnd, &MousePos);

    // 스크린 -> NDC
    // TODO. 뷰포트를 가져와서 크기를 얻어내야 함.
    UINT Width = GetEngine()->GetWorldBoundedWindow(this)->GetWidth<UINT>();
    UINT Height = GetEngine()->GetWorldBoundedWindow(this)->GetHeight<UINT>();
    Vec3 NearNdc = {}, FarNdc = {};
    NearNdc.x = FarNdc.x = MousePos.x / (Width / 2.f) - 1.f;
    NearNdc.y = FarNdc.y = MousePos.y / -(Height / 2.f) + 1.f;
    NearNdc.z = GraphicDevice::GetNear();
    FarNdc.z = GraphicDevice::GetFar();

    // NDC -> 투영 -> 뷰 -> 월드
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

    EPrimitiveType PrimitiveType = static_cast<EPrimitiveType>(InPrimitiveType);

    uint32 DataNum = GetSize(InPrimitives);
    for (uint32 DataIndex = 0; DataIndex < DataNum; ++DataIndex)
    {
        XMMATRIX XMWorldMat = {};
        XMMATRIX XMInverseWorldMat = {};

        auto& PrimitiveData = InPrimitives[DataIndex];
        if (PrimitiveData.PrimitiveType != PrimitiveType)
        {
            continue;
        }
        
        if (std::shared_ptr<MPrimitiveComponent> PrimitiveComponent = PrimitiveData.PrimitiveComponent.lock())
        {
            if (PrimitiveComponent->getRenderMdoe() == MPrimitiveComponent::ERenderMode::Orthogonal)
            {
                continue;
            }

            XMWorldMat = XMLoadFloat4x4(&PrimitiveComponent->getWorldMatrix());
            XMInverseWorldMat = XMLoadFloat4x4(&PrimitiveComponent->GetInverseWorldMatrix());
        }
        else
        {
            Mat4 WorldMat = {};
            TransformMatrix(WorldMat, PrimitiveData.Scale, VEC3ZERO, PrimitiveData.Translation);

            XMWorldMat = XMLoadFloat4x4(&WorldMat);
            XMInverseWorldMat = XMMatrixInverse(nullptr, XMWorldMat);
        }

        XMVECTOR Start = XMVector3TransformCoord(NearWorldPos, XMInverseWorldMat);
        XMVECTOR End = XMVector3TransformCoord(FarWorldPos, XMInverseWorldMat);
        XMVECTOR Dir = XMVector3Normalize(End - Start);

        if (XMVectorGetX(XMVectorIsNaN(Start)) != 0 || XMVectorGetX(XMVectorIsNaN(End)) != 0)
        {
            continue;
        }

        const auto& MeshData = *PrimitiveData.MeshData;
        const auto& Vertices = MeshData.Vertices;
        const auto& Indices = MeshData.Indices;

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
                XMVECTOR WorldHitPos = XMVector3TransformCoord(LocalHitPos, XMWorldMat);

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

    return OutHitData.IsValid();
}

bool MWorld::IsForegorund() const
{
    if (auto& Window = GetEngine()->GetWorldBoundedWindow(this))
    {
        return Window->IsForegorund();
    }

    return false;
}

bool MWorld::IsMouseInViewport() const
{
    if (auto& Window = GetEngine()->GetWorldBoundedWindow(this))
    {
        return Window->IsMouseInViewport();
    }

    return false;
}

void MWorld::ScreenToWorld(const Vec2& InPos, float Depth, Vec3& OutPos) const
{
    auto Window = GetEngine()->GetWorldBoundedWindow(this);
    // 스크린 -> NDC
    float Width = Window->GetWidth<float>();
    float Height = Window->GetHeight<float>();
    
    Vec3 NDC = {
        InPos.x / (Width / 2.f) - 1.f,
        InPos.y / -(Height / 2.f) + 1.f,
        Depth
    };

    XMVECTOR ViewPos = XMVector3TransformCoord(XMLoadFloat3(&NDC), XMLoadFloat4x4(&getMainCamera()->getInverseProjectionMatrix()));
    XMVECTOR WorldPos = XMVector3TransformCoord(ViewPos, XMLoadFloat4x4(&getMainCamera()->getInvesrViewMatrix()));

    XMStoreFloat3(&OutPos, WorldPos);
}

void MWorld::WorldToScreen(const Vec3& InPos, Vec2& OutPos) const
{
    auto Window = GetEngine()->GetWorldBoundedWindow(this);
    // 스크린 -> NDC
    float Width = Window->GetWidth<float>();
    float Height = Window->GetHeight<float>();

    XMVECTOR ViewPos = XMVector3TransformCoord(XMLoadFloat3(&InPos), XMLoadFloat4x4(&getMainCamera()->getViewMatrix()));
    XMVECTOR ProjectPos = XMVector3TransformCoord(ViewPos, XMLoadFloat4x4(&getMainCamera()->getProjectionMatrix()));
    //XMVECTOR NDCPos = ProjectPos / XMVectorGetZ(ProjectPos); XMVector3TransformCoord가 z나누기 해줌

    OutPos = { (XMVectorGetX(ProjectPos) + 1.f) * (Width / 2.f), (-XMVectorGetY(ProjectPos) + 1.f) * (Height / 2.f)};
}

void MWorld::ProjectVec3(const Vec3& InBase, const Vec3& InTarget, Vec3& Out) const
{
    XMVECTOR Base = XMVector3Normalize(XMLoadFloat3(&InBase));
    XMVECTOR Target = XMLoadFloat3(&InTarget);

    float Dot = XMVectorGetX(XMVector3Dot(Base, Target));

    XMVECTOR Proj = Base * (Dot * XMVector3Length(Target));
    XMStoreFloat3(&Out, Proj);
}

void MWorld::ProjectVec2(const Vec2& InBase, const Vec2& InTarget, Vec2& Out) const
{
    XMVECTOR Base = XMVector2Normalize(XMLoadFloat2(&InBase));
    XMVECTOR Target = XMLoadFloat2(&InTarget);

    float Dot = XMVectorGetX(XMVector2Dot(Base, Target));

    XMVECTOR Proj = Base * (Dot * XMVector2Length(Target));
    XMStoreFloat2(&Out, Proj);
}

std::shared_ptr<MActor> CreateActor(std::shared_ptr<MWorld> InWorld, const FTypeDesc* InTypeDesc)
{
    assert(InWorld);
    assert(InTypeDesc);
    assert(InTypeDesc->IsA<MActor>());

    std::shared_ptr<MActor> NewActor(static_cast<MActor*>(CreateObject(InTypeDesc)));
    NewActor->SetOwner(InWorld);
    NewActor->PostConstruct();
    InWorld->addActor(NewActor);

    return NewActor;
}
