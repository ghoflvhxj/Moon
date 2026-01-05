#include "Renderer.h"
#include "MoonEngine.h"

#include "MapUtility.h"
#include "Utility/PerformanceTimer.h"

// DirectXTK
#include "DirectXTK/SpriteFont.h"

#include "WindowManager.h"
#include "Window.h"

// Graphic
#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

#include "RenderTarget.h"
#include "RenderPass.h"
#include "CombinePass.h"
#include "Module/Render/RenderPass/FullScreenQuadPass.h"

#include "Material.h"
#include "Shader.h"

#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"

// Framework
#include "World.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "MeshComponent.h"
#include "LightComponent.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"

#include "Texture.h"

#include "Camera.h"

#include "Core/ResourceManager.h"

#undef max
#undef min

using namespace DirectX;

#define MinimalRendering 0
#define RenderPassPerformanceProfiling 0

constexpr wchar_t* CoordinateKey = TEXT("Coordinate");
constexpr wchar_t* CapsuleKey = TEXT("Capsule");

enum class EFrustumCascade
{
    Near,
    Middle,
    Middle2,
    Far,
    Count
};

MRenderer::MRenderer() noexcept
{
	//_renderTargets.reserve(CastValue<size_t>(ERenderTarget::Count));
	RenderPasses.resize(CastValue<size_t>(ERenderPass::End), nullptr);

    GetLevelChangedDelegate().Add([&]() {
        //PrimitiveDatasPerType.clear();
        //PrimitiveDatas.clear();

        GetScene(GetMainWorld()->GetID())->Clear();
    });
}

MRenderer::~MRenderer() noexcept
{
    Release();
}

bool MRenderer::Initialize()
{
    Super::Initialize();

    GetEngine()->GetOnWorldAddedDelegate().Add(this, &MRenderer::AddScene);

    // BindRenderTargets 컴파일 성공용. 제거해야함
    RenderTargets _renderTargets;

    RenderPasses[EnumToIndex(ERenderPass::ZPre)] = CreateRenderPass<MDepthPre>();
    {
        RenderPasses[EnumToIndex(ERenderPass::ZPre)]->SetDefaultShader(TEXT("TexAnimVertexShader.cso"), nullptr);
        RenderPasses[EnumToIndex(ERenderPass::ZPre)]->ApplyDefaultShaderOnly(true);
    }

#if MinimalRendering == 0
    RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)] = CreateRenderPass<DirectionalShadowDepthPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->BindRenderTargets(_renderTargets,
            ERenderTarget::DirectionalShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->SetDefaultShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPixel.cso"), TEXT("ShadowDepthGS.cso"));
        RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->Color = EngineColors::White;
    }

    RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)] = CreateRenderPass<PointShadowDepthPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->BindRenderTargets(_renderTargets,
            ERenderTarget::PointShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->SetDefaultShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPointPS.cso"), TEXT("ShadowDepthPointGS.cso"));
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->Color = EngineColors::White;
    }
#endif

    RenderPasses[EnumToIndex(ERenderPass::Geometry)] = CreateRenderPass<GeometryPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::RimLight
        );
    }

#if MinimalRendering == 0
    RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)] = CreateRenderPass<DirectionalLightPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)]->BindRenderTargets(_renderTargets,
            ERenderTarget::LightDiffuse,
            ERenderTarget::LightSpecular
        );

        RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)]->BindResourceViews(_renderTargets,
            ERenderTarget::Depth,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::DirectionalShadowDepth,
            ERenderTarget::RimLight
        );
    }

    RenderPasses[EnumToIndex(ERenderPass::PointLight)] = CreateRenderPass<PointLightPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->BindRenderTargets(_renderTargets,
            ERenderTarget::PointLightDiffuse,
            ERenderTarget::LightSpecular);
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->BindResourceViews(_renderTargets,
            ERenderTarget::Depth,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::PointShadowDepth
        );
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->SetClearTargets(false);
    }

    RenderPasses[EnumToIndex(ERenderPass::SkyPass)] = CreateRenderPass<SkyPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SkyPass)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::LightDiffuse
        );

        RenderPasses[EnumToIndex(ERenderPass::SkyPass)]->SetClearTargets(false);
    }

    RenderPasses[EnumToIndex(ERenderPass::Line)] = CreateRenderPass<MLinePass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Line)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Collision
        );

        RenderPasses[EnumToIndex(ERenderPass::Line)]->SetDefaultShader(TEXT("VS_Collision.cso"), TEXT("PS_Collision.cso"));
    }

    RenderPasses[EnumToIndex(ERenderPass::Outline)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Outline)]->BindResourceViews(_renderTargets,
            ERenderTarget::Stencil
        );
        RenderPasses[EnumToIndex(ERenderPass::Outline)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Outline
        );

        RenderPasses[EnumToIndex(ERenderPass::Outline)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_Outline.cso"));
        RenderPasses[EnumToIndex(ERenderPass::Outline)]->SetDepthEnable(false);
    }
#endif

    RenderPasses[EnumToIndex(ERenderPass::Combine)] = CreateRenderPass<MCombinePass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Combine)]->BindResourceViews(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::LightDiffuse,
            ERenderTarget::LightSpecular,
            ERenderTarget::Collision,
            ERenderTarget::PointLightDiffuse,
            ERenderTarget::Outline
        );

        RenderPasses[EnumToIndex(ERenderPass::Combine)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));
    }


    //addRenderTargetForDebug(ERenderTarget::DepthPre);
    DebugRenderTarget(ERenderTarget::Diffuse);
    DebugRenderTarget(ERenderTarget::Depth);
    DebugRenderTarget(ERenderTarget::Normal);
    DebugRenderTarget(ERenderTarget::Specular);
    DebugRenderTarget(ERenderTarget::LightDiffuse);
    DebugRenderTarget(ERenderTarget::LightSpecular);
    //DebugRenderTarget(ERenderTarget::DirectionalShadowDepth);
    //addRenderTargetForDebug(ERenderTarget::PointShadowDepth);
    DebugRenderTarget(ERenderTarget::Outline);
    DebugRenderTarget(ERenderTarget::Stencil);
    DebugRenderTarget(ERenderTarget::Collision);

    Mesh::MakeSphere(SphereMesh, 16);

    Mesh::MakeCoordinate(CoordinateMesh);
    getGraphicDevice()->BuildMeshBuffer(CoordinateKey, CoordinateMesh, 0);

    return EnumToIndex(ERenderPass::End) == GetSize(RenderPasses);
}

void MRenderer::Release()
{
    Super::Release();

    //_renderTargets.clear();
    RenderTargetss.clear();
    RenderPasses.clear();

    DebugRenderTargetMehses.clear();

    //PrimitiveDatasPerType.clear();

    //PrimitiveDatas.clear();
}

void MRenderer::DrawCylinder(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation)
{

}

void MRenderer::DrawSphere(float InRadius, const Vec3& InTranslation, const DirectX::XMVECTORF32& InColor)
{

}

void MRenderer::DrawCapsule(MWorld* InWorld, float InRadius, float InHalfHeight, const Vec3& InTranslation, const Vec3& InRotation)
{
    DrawCapsule(InWorld, InRadius, InHalfHeight, InTranslation, EulerToQuaternion(InRotation));
}

void MRenderer::DrawCapsule(MWorld* InWorld, float InRadius, float InHalfHeight, const Vec3& InTranslation, const Vec4& InQuatRotation)
{
    // TODO. 
    // 캡슐마다 반지름과 높이가 다르니 캡슐 그릴 때 만들어줌
    // 이 둘을 상수 버퍼로 전달할 수 있도록 개선하고, 이 코드는 제거해야 함.
    int Radius = static_cast<int>(InRadius * 10000.f);
    int Height = static_cast<int>(InHalfHeight * 10000.f);
    auto& TupleKey = std::make_tuple(Radius, Height);
    std::wstring BufferKey = CapsuleKey + std::to_wstring(Radius) + TEXT("_") + std::to_wstring(Height);
    if (CapsuleMeshDatas.find(TupleKey) == CapsuleMeshDatas.end())
    {
        FMeshData NewCapsuleMeshData = {};
        Mesh::MakeCapsule(NewCapsuleMeshData, InHalfHeight, InRadius);

        CapsuleMeshDatas[TupleKey] = NewCapsuleMeshData;

        
        getGraphicDevice()->BuildMeshBuffer(BufferKey, CapsuleMeshDatas[TupleKey], 0);
    }

    //if (CapsuleMeshData.Vertices.empty())
    //{
    //    Mesh::MakeCapsule(CapsuleMeshData, InHalfHeight, InRadius);
    //    getGraphicDevice()->BuildMeshBuffer(CapsuleKey, CapsuleMeshData, 0);
    //}

    static std::shared_ptr<MMaterial> Mat = std::make_shared<MMaterial>();
    Mat->setShader(TEXT("VS_VertexColorOut.cso"), TEXT("PS_Collision.cso"));
    Mat->setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    FPrimitiveData NewPrimitiveData = {};
    NewPrimitiveData.MeshData = &CapsuleMeshDatas[TupleKey];
    NewPrimitiveData.PrimitiveType = EPrimitiveType::CustomPrimitiveType0;
    NewPrimitiveData.Material = Mat;

    NewPrimitiveData.Translation = InTranslation;
    NewPrimitiveData.Rotation = InQuatRotation;
    NewPrimitiveData.Scale = VEC3ONE;

    FBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, BufferKey);
    NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[0];
    NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[0];

    Scenes[InWorld->GetID()]->DrawPrimitive(NewPrimitiveData);
}

void MRenderer::DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec4& InQuatRotation, const Vec3& InScale)
{
    static std::shared_ptr<MMaterial> Mat = std::make_shared<MMaterial>();
    Mat->setShader(TEXT("VS_VertexColorOut.cso"), TEXT("PS_Collision.cso"));
    Mat->setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    FPrimitiveData NewPrimitiveData = {};
    NewPrimitiveData.MeshData = &CoordinateMesh;
    NewPrimitiveData.PrimitiveType = EPrimitiveType::CustomPrimitiveType0;
    NewPrimitiveData.Material = Mat;

    NewPrimitiveData.Translation = InTranslation;
    NewPrimitiveData.Rotation = InQuatRotation;
    NewPrimitiveData.Scale = InScale;

    FBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, CoordinateKey);
    NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[0];
    NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[0];
    NewPrimitiveData.InstanceBuffer = Buffers.InstanceBuffers[0];

    Scenes[InWorld->GetID()]->DrawPrimitive(NewPrimitiveData);
}

void MRenderer::DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale)
{
    DrawCoordinate(InWorld, InTranslation, EulerToQuaternion(InRotation), InScale);
}

void MRenderer::DrawPrimitive(MWorld* InWorld, const std::shared_ptr<StaticMesh>& InMesh, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale, EPrimitiveType InPrimitiveType)
{
    uint32 WorldID = InWorld->GetID();
    getGraphicDevice()->BuildMeshSharedBuffers(InMesh);

    uint32 Num = InMesh->GetMeshNum();
    for (uint32 i = 0; i < Num; ++i)
    {
        FPrimitiveData NewPrimitiveData = {};

        NewPrimitiveData.MeshData = &InMesh->GetMeshData(i);
        NewPrimitiveData.Material = InMesh->getMaterial(0);
        NewPrimitiveData.PrimitiveType = InPrimitiveType;

        NewPrimitiveData.Translation = InTranslation;
        NewPrimitiveData.Rotation = EulerToQuaternion(InRotation);
        NewPrimitiveData.Scale = InScale;

        FBufferContainer Buffers;
        getGraphicDevice()->GetBuffers(Buffers, InMesh);
        NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[i];
        NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[i];

        Scenes[WorldID]->DrawPrimitive(NewPrimitiveData);
    }
}

void MRenderer::Test(uint32 InWorldID, uint32 InPID, std::shared_ptr<MMesh> InMesh)
{
    for (auto& [SceneID, Scene] : Scenes)
    {
        if (SceneID == InWorldID)
        {
            Scene->UpdateBuffer(InPID, InMesh);
        }
    }
}

MScene* MRenderer::GetCurrentScene()
{
    return Scenes[CurrentSceneID].get();
}

MScene* MRenderer::GetScene(uint32 InWorldID)
{
    auto& Iter = Scenes.find(InWorldID);
    if (Iter != Scenes.end())
    {
        return Iter->second.get();
    }

    auto& Iter2 = ScenesQueue.find(InWorldID);
    if (Iter2 != ScenesQueue.end())
    {
        return Iter2->second.get();
    }

    return nullptr;
}

//
//uint32 MRenderer::DrawVertices(const FMeshData& InMeshData)
//{
//	std::vector<FPrimitiveData> NewPrimitiveDatas;
//	FPrimitiveData NewPrimitivData = {};
//	NewPrimitivData.MeshData = &InMeshData;
//	NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
//	uint32 PrimitiveID = MPrimitiveComponent::MakePrimitiveID();
//	MakeBuffer(PrimitiveID, InMeshData);
//	NewPrimitivData.VertexBuffer = VertexBuffers[PrimitiveID][0];
//	NewPrimitivData.IndexBuffer = IndexBuffers[PrimitiveID][0];
//    NewPrimitiveDatas.push_back(NewPrimitivData);
//    PrimitiveDatas.emplace(PrimitiveID, NewPrimitiveDatas);
//
//	return PrimitiveID;
//}
//
//uint32 MRenderer::DrawLine(const std::vector<Vec3>& InWorldPositions)
//{
//    FMeshData NewMeshData = {};
//    NewMeshData.Vertices.reserve(InWorldPositions.size());
//    for (auto& InPosition : InWorldPositions)
//    {
//        Vertex NewVertex = {};
//        NewVertex.Pos.x = InPosition.x;
//        NewVertex.Pos.y = InPosition.y;
//        NewVertex.Pos.z = InPosition.z;
//
//        NewMeshData.Vertices.push_back(NewVertex);
//        NewMeshData.Indices.push_back(GetSize(NewMeshData.Vertices) - 1);
//    }
//
//    std::vector<FPrimitiveData> NewPrimitiveDatas;
//    FPrimitiveData NewPrimitivData = {};
//    NewPrimitivData.MeshData = &NewMeshData;
//    NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
//    uint32 PrimitiveID = MPrimitiveComponent::MakePrimitiveID();
//    MakeBuffer(PrimitiveID, NewMeshData);
//    NewPrimitivData.VertexBuffer = VertexBuffers[PrimitiveID][0];
//    NewPrimitivData.IndexBuffer = IndexBuffers[PrimitiveID][0];
//    NewPrimitiveDatas.push_back(NewPrimitivData);
//    PrimitiveDatas.emplace(PrimitiveID, NewPrimitiveDatas);
//
//    return PrimitiveID;
//}

void MRenderer::AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent)
{
	if (InPrimitiveComponent == nullptr)
	{
		return;
	}

    if (getGraphicDevice() == nullptr)
    {
        return;
    }

    InPrimitiveComponent->GetPrimitiveChangedDelegate().Add(this, &MRenderer::UpdatePrimitiveData);
    UpdatePrimitiveData(InPrimitiveComponent.get());
}

void MRenderer::RemovePrimitiveComponent(MPrimitiveComponent* InComponent)
{
    if (auto& Actor = InComponent->getOwningActor())
    {
        if (auto& Owner = Actor->GetOwner())
        {
            if (auto& World = Owner->CastToShared<MWorld>())
            {
                GetScene(World->GetID())->ClearPrimtiveDatas(InComponent->GetPrimitiveID());
            }
        }
    }
}

void MRenderer::UpdatePrimitiveData(MPrimitiveComponent* InComponent)
{
    uint32 PrimitiveID = InComponent->GetPrimitiveID();
    auto Scene = GetScene(InComponent->GetWorld()->GetID());
    Scene->ClearPrimtiveDatas(PrimitiveID);

    // Component로부터 PrimitiveData 생성
    std::vector<FPrimitiveData> NewPrimitiveDatas;
    if (InComponent->GetPrimitiveData(NewPrimitiveDatas) == false)
    {
        return;
    }

    Scene->AddPrimitiveDatas(PrimitiveID, NewPrimitiveDatas);

    std::shared_ptr<MMesh> Mesh = nullptr;
    if (auto& MeshComp = InComponent->CastToShared<MMeshComponent>())
    {
        Mesh = MeshComp->GetMesh();
    }
    else if (auto& LightComp = InComponent->CastToShared<MLightComponent>())
    {
        Mesh = LightComp->GetMesh();
    }

    if (Mesh != nullptr)
    {
        getGraphicDevice()->BuildMeshBuffersFromComponent(PrimitiveID, Mesh);
        Scene->UpdateBuffer(PrimitiveID, Mesh);
    }

    //auto Scene = GetScene(InComponent->GetWorld()->GetID());
    //Scene->UpdatePrimitiveData(InComponent);
}

const std::vector<const FPrimitiveData*>& MRenderer::GetPrimitiveDatas(EPrimitiveType InPrimitiveType)
{
    return Scenes[CurrentSceneID]->GetPrimitiveDatas(InPrimitiveType);
}

void MRenderer::AddRenderTargets(uint32 InWidth, uint32 InHeight)
{
    RenderTargets NewRenderTargets;

    // 렌더 타겟 추가
    for (int i = 0; i < CastValue<int>(ERenderTarget::Count); ++i)
    {
        FRenderTagetInfo RenderTargetInfo;

        switch (CastValue<ERenderTarget>(i))
        {
        case ERenderTarget::Diffuse:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
        }
        break;
        case ERenderTarget::Depth:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Depth;
        }
        break;
        case ERenderTarget::Normal:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Normal;
        }
        break;
        case ERenderTarget::LightDiffuse:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Light;
        }
        break;
        case ERenderTarget::DirectionalShadowDepth:
        {
            RenderTargetInfo.bCube = false;
            RenderTargetInfo.Width = 1024 * 2;
            RenderTargetInfo.Height = 1024 * 2;
            RenderTargetInfo.TextrueNum = CastValue<int>(EFrustumCascade::Count);
            RenderTargetInfo.Type = ERenderTargetType::Depth;
        }
        break;
        case ERenderTarget::PointShadowDepth:
        {
            static constexpr uint32 MaxPointLightNum = 10;
            RenderTargetInfo = FRenderTagetInfo::GetCube();
            RenderTargetInfo.Width = 1024 * 2;
            RenderTargetInfo.Height = 1024 * 2;
            RenderTargetInfo.TextrueNum *= MaxPointLightNum;
            RenderTargetInfo.Type = ERenderTargetType::Depth;
        }
        break;
        case ERenderTarget::RimLight:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Bool;
        }
        break;
        default:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
        }
        break;
        }

        auto& NewRenderTarget = std::make_shared<MRenderTarget>();
        NewRenderTarget->initializeTexture(RenderTargetInfo);
        NewRenderTargets.push_back(std::move(NewRenderTarget));
    }

    RenderTargetss[std::make_tuple(InWidth, InHeight)] = std::move(NewRenderTargets);
}

void MRenderer::ResizeRenderTargets(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight, bool InFullScreen)
{
    std::vector<std::shared_ptr<MWindow>> Windows;
    GetWindowManager()->FilterWindow([InOldWidth, InOldHeight](std::shared_ptr<MWindow> Window) {
        return Window->GetWidth<uint32>() == InOldWidth && Window->GetHeight<uint32>() == InOldHeight;
    }, Windows);

    if (Windows.empty())
    {
        RenderTargetss.erase(std::make_tuple(InOldWidth, InOldHeight));
    }

    AddRenderTargets(InNewWidth, InNewHeight);
}

void MRenderer::DebugRenderTarget(ERenderTarget InRenderTarget)
{
    /*
#ifdef _DEBUG
    std::shared_ptr<MMaterial> BaseMat = nullptr;
    g_ResourceManager->Load(TEXT("Base/RenderTarget.json"), BaseMat);

	float scale = 200.f;
	float x = (-1.f * g_pSetting->getResolutionWidth<float>() / 2.f) + (scale / 2.f);
	float y = g_pSetting->getResolutionHeight<float>();

    auto& MeshComp = std::make_shared<StaticMeshComponent>();
	MeshComp->SetPhysics(false);
    MeshComp->SetMesh(TEXT("Base/Plane.fbx"));
    if (MapUtility::FindInsert(DebugRenderTargetMehses, InRenderTarget, MeshComp))
    {
        uint32 count = CastValue<uint32>(DebugRenderTargetMehses.size() - 1);
        MeshComp->setScale(scale, scale, 0.f);
        MeshComp->setTranslation(x + scale * count, scale, 1.f);

        std::shared_ptr<MMaterial> NewMat = std::make_shared<MMaterial>();
        *NewMat.get() = *BaseMat.get();// 텍스쳐가 다 달라서...
        NewMat->setTexture(ETextureType::Diffuse, GetRenderTarget(InRenderTarget)->AsTexture()); 

		MeshComp->SetMaterial(0, NewMat);
        MeshComp->setRenderMode(MPrimitiveComponent::ERenderMode::Orthogonal);
		MeshComp->SceneComponent::Update(0.f);

        AddPrimitiveComponentTemp(MeshComp);
    }
#endif
    */
}

std::shared_ptr<MRenderTarget> MRenderer::GetRenderTarget(ERenderTarget InRenderTarget)
{
    if (std::shared_ptr<MWindow>& Window = GetCurrentScene()->GetWindow())
    {
        auto& Iter = RenderTargetss.find(std::make_tuple(Window->GetWidth<uint32>(), Window->GetHeight<uint32>()));
        if (Iter != RenderTargetss.end())
        {
            return Iter->second[EnumToIndex(InRenderTarget)];
        }
    }

    return nullptr;
}

ID3D11ShaderResourceView* MRenderer::GetResourceView(ERenderTarget InRenderTarget)
{
    switch (InRenderTarget)
    {
    case ERenderTarget::Depth:
        return getGraphicDevice()->GetDepthResourceView();
    case ERenderTarget::Stencil:
        return getGraphicDevice()->GetStencilResourceView();
    default:
        return GetRenderTarget(InRenderTarget)->AsTexture()->getRawResourceViewPointer();
    }
}

void MRenderer::AddRenderPass(ERenderPass InRenderPassIndex, const std::shared_ptr<MRenderPass>& InRenderPass)
{
    if (InRenderPass == nullptr)
    {
        return;
    }

    switch (InRenderPassIndex)
    {
    case ERenderPass::CustomPass0:
    case ERenderPass::CustomPass1:
    case ERenderPass::CustomPass2:
    case ERenderPass::CustomPass3:
        RenderPasses[(int)InRenderPassIndex] = InRenderPass;
        break;
    default:
        break;
    }
}

void MRenderer::Render()
{
    /*
    // 스피어 그리기
    PrimitiveDatas[SpherePID].clear();
    if (SphereRenderDatas.empty() == false)
    {
        std::vector<FVertex_Instance> InstanceDatas;
        for (auto& SphereRenderData : SphereRenderDatas)
        {
            FVertex_Instance NewInstance = {};
            XMMATRIX XMWorldMat = XMMatrixScalingFromVector(XMLoadFloat3(&SphereRenderData.Scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&SphereRenderData.Translation));
            XMStoreFloat4x4(&NewInstance.WorldMatrix, XMWorldMat);
            InstanceDatas.push_back(NewInstance);
        }

        std::vector<FPrimitiveData> NewPrimitiveDatas;
        FPrimitiveData NewPrimitivData = {};
        NewPrimitivData.MeshData = &SphereMesh;
        NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
        NewPrimitivData.VertexBuffer = VertexBuffers[SpherePID][0];
        NewPrimitivData.IndexBuffer = IndexBuffers[SpherePID][0];
        NewPrimitivData.InstanceBuffer = InstanceBuffer;
        NewPrimitivData.InstanceNum = GetSize(SphereRenderDatas);

        InstanceBuffer->Update(InstanceDatas.data(), NewPrimitivData.InstanceNum);
        NewPrimitiveDatas.push_back(NewPrimitivData);

        PrimitiveDatas[SpherePID].insert(PrimitiveDatas[SpherePID].end(), NewPrimitiveDatas.begin(), NewPrimitiveDatas.end());
    }
    SphereRenderDatas.clear();
    */
    
    /*
    std::vector<FPrimitiveData> PostRenderPrimitiveDatas;

#ifdef _DEBUG
    // 렌더 타겟
    if (true == bDebugRenderTargets)
    {
        for (auto pair : DebugRenderTargetMehses)
        {
            auto& RenderTargetMesh = pair.second;
            uint32 PID = RenderTargetMesh->GetPrimitiveID();
            RenderTargetMesh->GetPrimitiveData(PostRenderPrimitiveDatas);
            PostRenderPrimitiveDatas.back().VertexBuffer = VertexBuffers[PID][0];
            PostRenderPrimitiveDatas.back().IndexBuffer = IndexBuffers[PID][0];
        }
    }
#endif
    */
}

void MRenderer::AddScene(const FWorldRenderInfo& InWorldRenderInfo)
{
    assert(InWorldRenderInfo.DstWindow);
    assert(InWorldRenderInfo.SrcWorld);

    uint32 WorldID = InWorldRenderInfo.SrcWorld->GetID();

    std::unique_ptr<MScene> NewScene = std::make_unique<MScene>();
    NewScene->SetWorld(InWorldRenderInfo.SrcWorld);
    Scenes[WorldID] = std::move(NewScene);

    auto& Window = InWorldRenderInfo.DstWindow;

    Window->GetOnViewportSizeChangedDelegate().Add([this, WorldID](uint32, uint32, uint32, uint32 NewWidth, uint32 NewHeight, bool) {
        const Vec3 NewScale = { static_cast<float>(NewWidth), static_cast<float>(NewHeight), 0.f};
        for (auto& PrimitiveData : GetScene(WorldID)->GetPrimitiveDatas(EPrimitiveType::DirectionalLight))
        {
            std::shared_ptr<MLightComponent> LightComp = PrimitiveData->GetPrimitiveComponent<MLightComponent>();
            assert(LightComp);

            LightComp->UpdateSize(NewScale.x, NewScale.y);
        }
        for (auto& PrimitiveData : GetScene(WorldID)->GetPrimitiveDatas(EPrimitiveType::PointLight))
        {
            std::shared_ptr<MLightComponent> LightComp = PrimitiveData->GetPrimitiveComponent<MLightComponent>();
            assert(LightComp);

            LightComp->UpdateSize(NewScale.x, NewScale.y);
        }
    });

    ResizeRenderTargets(Window->GetID(), 0, 0, Window->GetWidth<uint32>(), Window->GetHeight<uint32>(), Window->IsFullScreen());
    Window->GetOnViewportSizeChangedDelegate().Add(this, &MRenderer::ResizeRenderTargets);
}

void MRenderer::RenderWorld(const std::shared_ptr<MWorld>& InWorld)
{
    if (InWorld == nullptr)
    {
        return;
    }

    if (InWorld->getMainCamera() == nullptr)
    {
        return;
    }

    CurrentSceneID = InWorld->GetID();
    auto& Scene = Scenes[CurrentSceneID];

    // RenderScene에서 여기로 옮김
    // 그래야 GetPrimitiveDatas가 정상동작함
    Scene->Begin();

    // Cascade Shadow
    auto& DirectionalLightPrimitiveDatas = Scene->GetPrimitiveDatas(EPrimitiveType::DirectionalLight);
    if (DirectionalLightPrimitiveDatas.empty() == false)
    {
        const std::shared_ptr<MLightComponent>& LightComponent = DirectionalLightPrimitiveDatas[0]->GetPrimitiveComponent<MLightComponent>();

        auto& Window = Scene->GetWindow();
        float AspectRatio = Window->GetAspectRatio();
        if (std::isnan(AspectRatio))
        {
            return;
        }

        auto& Camera = InWorld->getMainCamera();
        float tanHalfVertical = tanf(XMConvertToRadians(Camera->getFov() / 2.f));
        float tanHalfHorizen = tanHalfVertical * AspectRatio;

        //float tanHalfVertical = tanf(XMConvertToRadians(g_pSetting->getFov() / 2.f));
        //float tanHalfHorizen = tanHalfVertical * g_pSetting->getAspectRatio();

        XMMATRIX XMCameraWorldMat = XMLoadFloat4x4(&Camera->getInvesrViewMatrix());
        XMVECTOR LightDirection = XMVector3Normalize(XMLoadFloat3(&LightComponent->GetDirection()));
        XMVECTOR UpVector = XMLoadFloat3(&VEC3UP);
        if (fabs(XMVectorGetX(XMVector3Dot(UpVector, LightDirection))) > 0.999f)
        {
            UpVector = XMVectorSet(1.f, 0, 0, 0);
        }

        for (int cascadeIndex = 0; cascadeIndex < CastValue<int>(EFrustumCascade::Far); ++cascadeIndex)
        {
            float Near = Scene->GetCascadeDistance(cascadeIndex);
            float Far = Scene->GetCascadeDistance(cascadeIndex + 1);

            float XNear = Near * tanHalfHorizen;
            float XFar = Far * tanHalfHorizen;
            float YNear = Near * tanHalfVertical;
            float YFar = Far * tanHalfVertical;
            float DepthCenter = (Near + Far) / 2.f;

            std::vector<Vec3> FrustumVertices = {
                //near Face
                {XNear,YNear,Near},
                {-XNear,YNear,Near},
                {XNear,-YNear,Near},
                {-XNear,-YNear,Near},
                //far Face
                {XFar,YFar,Far},
                {-XFar,YFar,Far},
                {XFar,-YFar,Far},
                {-XFar,-YFar,Far}
            };

            XMVECTOR CascadeCenterInWorld = {};
            for (auto& FrustumVertex : FrustumVertices)
            {
                XMStoreFloat3(&FrustumVertex, XMVector3TransformCoord(XMLoadFloat3(&FrustumVertex), XMCameraWorldMat));
                CascadeCenterInWorld += XMLoadFloat3(&FrustumVertex);
            }
            CascadeCenterInWorld /= static_cast<float>(FrustumVertices.size());

            float Radius = 0.f;
            for (auto& vertex : FrustumVertices)
            {
                float Length = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vertex) - CascadeCenterInWorld));
                Radius = std::max<float>(Length, Radius);
            }
            Radius = std::ceil(Radius * 2.f) / 2.f;

            XMVECTOR Eye = CascadeCenterInWorld - (LightDirection * Radius);
            XMVECTOR Focus = CascadeCenterInWorld;
            XMMATRIX LightView = XMMatrixLookAtLH(Eye, Focus, UpVector);
            XMMATRIX OrthoProjMatrix = XMMatrixOrthographicOffCenterLH(-Radius, Radius, -Radius, Radius, 0.1f, Radius * 2.f);

            Scene->SetLightInfoCascadeShadow(cascadeIndex, Eye, LightView * OrthoProjMatrix);
        }
    }

    PerformanceTimer p(TEXT("SceneRenderTime: "));
    RenderScene(Scene);
    SceneRenderTime = p.Record();

    InWorld->GetOnRederedDelegate().Broadcast();
}

std::shared_ptr<MWorld> MRenderer::GetWorld()
{
    return Scenes[CurrentSceneID]->GetWorld();
}

void MRenderer::RenderScene(std::unique_ptr<MScene>& InScene)
{
    //InScene->Begin();

    FrustumCulling(InScene);

    const auto& RenderablePrimitiveDatas = InScene->GetRenderablePrimitiveData();

    Times.clear();

    PerformanceTimer t(TEXT("RenderPassTime: "));
    uint32 RenderPassNum = GetSize(RenderPasses);
    for (uint32 PassIndex = 0; PassIndex < RenderPassNum; ++PassIndex)
    {
        if (RenderPasses[PassIndex] == nullptr)
        {
            continue;
        }

        if (bDrawShadow)
        {
            if (PassIndex == EnumToIndex(ERenderPass::PointShadowDepth) || PassIndex == EnumToIndex(ERenderPass::ShadowDepth))
            {
                RenderPasses[PassIndex]->Clear();
                continue;
            }
        }

#if RenderPassPerformanceProfiling == 1
        std::wstring Name = TEXT("Pass") + std::to_wstring(PassIndex) + TEXT(" :");
        PerformanceTimer Temp(Name);
        g_pGraphicDevice->QueryStart(PassIndex);
#endif

        RenderPasses[PassIndex]->RenderPass(RenderablePrimitiveDatas);

#if RenderPassPerformanceProfiling == 1
        Times.push_back(Temp.Record());
        g_pGraphicDevice->QueryFinish(PassIndex);
#endif
    }
    RenderPassTime = t.Record();

    InScene->End();
}

void MRenderer::RenderText()
{

}

void MRenderer::FrustumCulling(std::unique_ptr<MScene>& InScene)
{
    std::vector<XMVECTOR> Planes;
    if (const std::shared_ptr<MCamera>& Camera = InScene->GetWorld()->getMainCamera())
    {
        Planes.resize(6);

        XMMATRIX ViewProj = XMMatrixMultiply(XMLoadFloat4x4(&Camera->getViewMatrix()), XMLoadFloat4x4(&Camera->getPerspectiveProjectionMatrix()));
        XMStoreFloat4x4(&ViewPerspectiveProjMatrix, ViewProj);

        XMMATRIX XMOrthoViewProject = XMLoadFloat4x4(&Camera->getViewMatrix()) * XMLoadFloat4x4(&Camera->getOrthographicProjectionMatrix());

        XMStoreFloat4x4(&ViewOrthogonalProjMatrix, XMOrthoViewProject);

        // 평면의 방정식 ax + by + cz + d = 0을 구해야 함
        // 절두체 Near 평면
        float a = ViewPerspectiveProjMatrix._13;
        float b = ViewPerspectiveProjMatrix._23;
        float c = ViewPerspectiveProjMatrix._33;
        float d = ViewPerspectiveProjMatrix._43;
        Planes[0] = XMVectorSet(a, b, c, d);
        Planes[0] = XMPlaneNormalize(Planes[0]);

        // 절두체 Far 평면
        a = (float)(ViewPerspectiveProjMatrix._14 - ViewPerspectiveProjMatrix._13);
        b = (float)(ViewPerspectiveProjMatrix._24 - ViewPerspectiveProjMatrix._23);
        c = (float)(ViewPerspectiveProjMatrix._34 - ViewPerspectiveProjMatrix._33);
        d = (float)(ViewPerspectiveProjMatrix._44 - ViewPerspectiveProjMatrix._43);
        Planes[1] = XMVectorSet(a, b, c, d);
        Planes[1] = XMPlaneNormalize(Planes[1]);

        // 절두체의 왼쪽 평면
        a = (float)(ViewPerspectiveProjMatrix._14 + ViewPerspectiveProjMatrix._11);
        b = (float)(ViewPerspectiveProjMatrix._24 + ViewPerspectiveProjMatrix._21);
        c = (float)(ViewPerspectiveProjMatrix._34 + ViewPerspectiveProjMatrix._31);
        d = (float)(ViewPerspectiveProjMatrix._44 + ViewPerspectiveProjMatrix._41);
        Planes[2] = XMVectorSet(a, b, c, d);
        Planes[2] = XMPlaneNormalize(Planes[2]);

        // 절두체의 오른쪽 평면
        a = (float)(ViewPerspectiveProjMatrix._14 - ViewPerspectiveProjMatrix._11);
        b = (float)(ViewPerspectiveProjMatrix._24 - ViewPerspectiveProjMatrix._21);
        c = (float)(ViewPerspectiveProjMatrix._34 - ViewPerspectiveProjMatrix._31);
        d = (float)(ViewPerspectiveProjMatrix._44 - ViewPerspectiveProjMatrix._41);
        Planes[3] = XMVectorSet(a, b, c, d);
        Planes[3] = XMPlaneNormalize(Planes[3]);

        // 절두체의 윗 평면
        a = (float)(ViewPerspectiveProjMatrix._14 - ViewPerspectiveProjMatrix._12);
        b = (float)(ViewPerspectiveProjMatrix._24 - ViewPerspectiveProjMatrix._22);
        c = (float)(ViewPerspectiveProjMatrix._34 - ViewPerspectiveProjMatrix._32);
        d = (float)(ViewPerspectiveProjMatrix._44 - ViewPerspectiveProjMatrix._42);
        Planes[4] = XMVectorSet(a, b, c, d);
        Planes[4] = XMPlaneNormalize(Planes[4]);

        // 절두체의 아래 평면
        a = (float)(ViewPerspectiveProjMatrix._14 + ViewPerspectiveProjMatrix._12);
        b = (float)(ViewPerspectiveProjMatrix._24 + ViewPerspectiveProjMatrix._22);
        c = (float)(ViewPerspectiveProjMatrix._34 + ViewPerspectiveProjMatrix._32);
        d = (float)(ViewPerspectiveProjMatrix._44 + ViewPerspectiveProjMatrix._42);
        Planes[5] = XMVectorSet(a, b, c, d);
        Planes[5] = XMPlaneNormalize(Planes[5]);

    }

    TotalPrimitiveNum = 0;
    for (const auto& [Id, PrimitiveDatas] : InScene->GetPrimitiveDatas())
    {
        if (PrimitiveDatas.empty())
        {
            continue;
        }

        TotalPrimitiveNum += GetSize(PrimitiveDatas);

        if (Planes.empty() == false)
        {
            if (const std::shared_ptr<MPrimitiveComponent>& PrimitiveComponent = PrimitiveDatas[0].PrimitiveComponent.lock())
            {
                std::shared_ptr<MBoundingBox> BoundingBox = nullptr;
                // TODO 바운딩 박스가 없으면 일단 무조건 렌더링
                //if (PrimitiveComponent->GetBoundingBox(BoundingBox))
                //{
                //    // 컬링
                //    if (BoundingBox->cullSphere(Planes, PrimitiveComponent->getWorldTranslation(), BoundingBox->GetLength(PrimitiveComponent->getScale()) / 2.f) == false)
                //    {
                //        CulledPrimitiveNum += GetSize(PrimitiveDatas);
                //        continue;
                //    }
                //    
                //}
            }
        } 

        InScene->AddRenderablePrimitiveDatas(PrimitiveDatas);
    }

    auto& TemporalPrimitiveDatas = InScene->GetTemporalPrimitiveDatas();
    if (TemporalPrimitiveDatas.empty() == false)
    {
        InScene->AddRenderablePrimitiveDatas(TemporalPrimitiveDatas);
    }
}

MScene::MScene()
    : CascadeDistances(4, 0.f)
    , CascadeLightPositions(3, VEC4ZERO)
    , CascadeLightMatrices(3, IDENTITYMATRIX)
{
    CascadeDistances[CastValue<int>(EFrustumCascade::Near)] = 0.1f;
    CascadeDistances[CastValue<int>(EFrustumCascade::Middle)] = 6.f;
    CascadeDistances[CastValue<int>(EFrustumCascade::Middle2)] = 18.f;
    CascadeDistances[CastValue<int>(EFrustumCascade::Far)] = 1000.f;
}

void MScene::Begin()
{
    RenderablePrimitiveData.clear();
    PrimitiveDatasPerType.clear();

    // Draw함수마다 매번 할 필요는 없고, 패스가 시작되기 전에 해주면 될듯
    for (auto& [InstanceBuffer, BufferInstanceDatas] : InstanceDatas)
    {
        InstanceBuffer->Update(BufferInstanceDatas.data(), GetSize(BufferInstanceDatas));
    }

    for (auto& [PID, PrimitiveDataList] : PrimitiveDatas)
    {
        for (auto& PrimitiveData : PrimitiveDataList)
        {
            PrimitiveDatasPerType[PrimitiveData.PrimitiveType].push_back(&PrimitiveData);
        }
    }

    UpdateGlobalConstantBuffer();
    UpdateTickConstantBuffer();
}

void MScene::End()
{
    CachedTemporalPrimitiveDatas = std::move(TemporalPrimitiveDatas);
    InstanceDatas.clear();
}

std::shared_ptr<MWindow> MScene::GetWindow() const
{
    return GetEngine()->GetWorldBoundedWindow(GetWorld().get());
}

void MScene::SetWorld(std::shared_ptr<MWorld> InWorld)
{
    World = InWorld;
}

std::shared_ptr<MWorld> MScene::GetWorld() const
{
    return World.lock();
}

void MScene::UpdateGlobalConstantBuffer()
{
    std::shared_ptr<MConstantBuffer>& GlobalBuffer = MShader::GetSharedConstantBuffer(EConstantBufferLayer::Global);
    if (GlobalBuffer == nullptr)
    {
        return;
    }

    Vec4 resolution = { GetWindow()->GetWidth<float>(),GetWindow()->GetHeight<float>(), 0.f, 0.f };
    GlobalBuffer->SetData(TEXT("resolution"), &resolution);

    BOOL bLight = TRUE;
    GlobalBuffer->SetData(TEXT("bLight"), &bLight);

    GlobalBuffer->Commit();

    // 후순위로 개선하기
    ID3D11Buffer* DX_Buffer = GlobalBuffer->getRaw();
    g_pGraphicDevice->getContext()->VSSetConstantBuffers(0, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(0, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->GSSetConstantBuffers(0, 1, &DX_Buffer);
}

void MScene::UpdateTickConstantBuffer()
{
    EConstantBufferLayer Layer = EConstantBufferLayer::Tick;
    uint32 LayerIndex = EnumToIndex(Layer);
    std::shared_ptr<MConstantBuffer>& TickBuffer = MShader::GetSharedConstantBuffer(Layer);
    if (TickBuffer == nullptr)
    {
        return;
    }

    TickBuffer->SetData(TEXT("cascadeDistance"), CascadeDistances.data());
    TickBuffer->SetData(TEXT("lightPos"), CascadeLightPositions.data());
    TickBuffer->SetData(TEXT("lightViewProjMatrix"), CascadeLightMatrices.data());

    TickBuffer->SetData(TEXT("viewMatrix"), &GetWorld()->getMainCameraViewMatrix());
    TickBuffer->SetData(TEXT("projectionMatrix"), &GetWorld()->getMainCameraProjectioinMatrix());
    TickBuffer->SetData(TEXT("identityMatrix"), &IDENTITYMATRIX);
    TickBuffer->SetData(TEXT("orthographicProjectionMatrix"), &GetWorld()->getMainCameraOrthographicProjectionMatrix());
    TickBuffer->SetData(TEXT("inverseOrthographicProjectionMatrix"), &GetWorld()->getMainCamera()->getInverseOrthographicProjectionMatrix());

    TickBuffer->Commit();

    // 후순위로 개선하기
    ID3D11Buffer* DX_Buffer = TickBuffer->getRaw();
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->VSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->GSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
}

void MScene::UpdateBuffer(uint32 InPID, std::shared_ptr<MMesh>& InMesh)
{
    uint32 PrimitiveDataNum = GetSize(PrimitiveDatas[InPID]);

    if (PrimitiveDataNum == 0)
    {
        // PrimitiveData의 Buffer를 채우는 함수인데, PrimitiveData가 없으면 안됨
        return;
    }

    FBufferContainer SharedBuffers = {};
    getGraphicDevice()->GetBuffers(SharedBuffers, InMesh);

    FBufferContainer PrivateBuffers = {};
    getGraphicDevice()->GetPrivateBuffers(PrivateBuffers, InPID);

    for (uint32 i = 0; i < PrimitiveDataNum; ++i)
    {
        FPrimitiveData& PrimitiveData = PrimitiveDatas[InPID][i];

        auto& Iter = PrivateBuffers.VertexBuffers.find(i);
        PrimitiveData.VertexBuffer = Iter == PrivateBuffers.VertexBuffers.end() ? SharedBuffers.VertexBuffers[i] : Iter->second;

        auto& Iter2 = PrivateBuffers.IndexBuffers.find(i);
        PrimitiveData.IndexBuffer = Iter2 == PrivateBuffers.IndexBuffers.end() ? SharedBuffers.IndexBuffers[i] : Iter2->second;

        //PrimitiveData.VertexBuffer = Buffers.VertexBuffers[i];
        //PrimitiveData.IndexBuffer = Buffers.IndexBuffers[i];
    }
}

void MScene::UpdatePrimitiveData(MPrimitiveComponent* InComponent)
{
    uint32 PrimitiveID = InComponent->GetPrimitiveID();
    ClearPrimtiveDatas(PrimitiveID);

    std::shared_ptr<MMesh> Mesh = nullptr;
    if (auto& MeshComp = InComponent->CastToShared<MMeshComponent>())
    {
        Mesh = MeshComp->GetMesh();
    }
    else if (auto& LightComp = InComponent->CastToShared<MLightComponent>())
    {
        Mesh = LightComp->GetMesh();
    }

    if (Mesh == nullptr)
    {
        return;
    }

    // Component로부터 PrimitiveData 생성
    std::vector<FPrimitiveData> NewPrimitiveDatas;
    if (InComponent->GetPrimitiveData(NewPrimitiveDatas))
    {
        AddPrimitiveDatas(PrimitiveID, NewPrimitiveDatas);
    }
}

void MScene::AddPrimitiveDatas(uint32 InPID, const std::vector<FPrimitiveData>& InPrimitiveDatas)
{
    PrimitiveDatas[InPID].insert(PrimitiveDatas[InPID].end(), InPrimitiveDatas.begin(), InPrimitiveDatas.end());
}

void MScene::ClearPrimtiveDatas(uint32 InPID)
{
    PrimitiveDatas[InPID].clear();
}

const std::vector<FPrimitiveData>& MScene::GetPrimitiveDatas(uint32 InPrimitiveID)
{
    auto& Iter = PrimitiveDatas.find(InPrimitiveID);
    if (Iter != PrimitiveDatas.end())
    {
        return PrimitiveDatas[InPrimitiveID];
    }

    return PrimitiveDatas[-1];
}

const std::map<uint32, std::vector<FPrimitiveData>>& MScene::GetPrimitiveDatas() const
{
    return PrimitiveDatas;
}

void MScene::AddRenderablePrimitiveDatas(const std::vector<FPrimitiveData>& InPrimitiveDatas)
{
    RenderablePrimitiveData.insert(RenderablePrimitiveData.end(), InPrimitiveDatas.begin(), InPrimitiveDatas.end());
}

const std::vector<FPrimitiveData>& MScene::GetRenderablePrimitiveData() const
{
    return RenderablePrimitiveData;
}

void MScene::DrawPrimitive(const FPrimitiveData& InPrimitiveData)
{
    if (auto& InstanceBuffer = InPrimitiveData.InstanceBuffer.lock())
    {
        // 한번은 PrimitiveData를 추가해야 함
        if (InstanceDatas.find(InstanceBuffer.get()) == InstanceDatas.end())
        {
            TemporalPrimitiveDatas.push_back(InPrimitiveData);
        }

        FVertex_Instance NewInstance = {};
        TransformMatrix(NewInstance.WorldMatrix, InPrimitiveData.Scale, InPrimitiveData.Rotation, InPrimitiveData.Translation);
        XMStoreFloat4x4(&NewInstance.WorldMatrix, XMLoadFloat4x4(&NewInstance.WorldMatrix)* XMLoadFloat4x4(&GetWorld()->getMainCameraViewMatrix())* XMLoadFloat4x4(&GetWorld()->getMainCameraProjectioinMatrix()));
        InstanceDatas[InstanceBuffer.get()].push_back(NewInstance);
    }
    else
    {
        TemporalPrimitiveDatas.push_back(InPrimitiveData);
    }
}

const std::vector<FPrimitiveData>& MScene::GetTemporalPrimitiveDatas() const
{
    return CachedTemporalPrimitiveDatas;
}

void MScene::SetLightInfoCascadeShadow(uint32 InIndex, const XMVECTOR& InLightPos, const XMMATRIX& InLightMat)
{
    XMStoreFloat4(&CascadeLightPositions[InIndex], InLightPos);
    XMStoreFloat4x4(&CascadeLightMatrices[InIndex], InLightMat);
    CascadeLightMatrices[InIndex]._42 = round(CascadeLightMatrices[InIndex]._42 * 10.f) / 10.f;
    CascadeLightMatrices[InIndex]._43 = round(CascadeLightMatrices[InIndex]._43 * 10.f) / 10.f;
}

//void MScene::Func()
//{
//    const std::shared_ptr<MCamera>& Camera = GetWorld()->getMainCamera();
//
//    XMMATRIX ViewProj = XMMatrixMultiply(XMLoadFloat4x4(&Camera->getViewMatrix()), XMLoadFloat4x4(&Camera->getPerspectiveProjectionMatrix()));
//    XMStoreFloat4x4(&ViewPerspectiveProjMatrix, ViewProj);
//
//    XMMATRIX XMOrthoViewProject = XMLoadFloat4x4(&Camera->getViewMatrix()) * XMLoadFloat4x4(&Camera->getOrthographicProjectionMatrix());
//    XMStoreFloat4x4(&ViewOrthogonalProjMatrix, XMOrthoViewProject);
//}