#include "Renderer.h"
#include "MoonEngine.h"

#include "MapUtility.h"
#include "Utility/PerformanceTimer.h"

// DirectXTK
#include "DirectXTK/SpriteFont.h"

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

enum class EFrustumCascade
{
    Near,
    Middle,
    Middle2,
    Far,
    Count
};

MRenderer::MRenderer() noexcept
	: CascadeDistances(4, 0.f)
    , CascadeLightPositions(3, VEC4ZERO)
    , CascadeLightMatrices(3, IDENTITYMATRIX)
{
	CascadeDistances[CastValue<int>(EFrustumCascade::Near)] = 0.1f;
	CascadeDistances[CastValue<int>(EFrustumCascade::Middle)] = 6.f;
	CascadeDistances[CastValue<int>(EFrustumCascade::Middle2)] = 18.f;
	CascadeDistances[CastValue<int>(EFrustumCascade::Far)] = 1000.f;

	_renderTargets.reserve(CastValue<size_t>(ERenderTarget::Count));
	RenderPasses.resize(CastValue<size_t>(ERenderPass::End), nullptr);

    GetLevelChangedDelegate().Add([&]() {
        //RenderablePrimitiveData.clear();
        PrimitiveDatasPerType.clear();

        PrimitiveComponents.clear();
        PrimitiveDatas.clear();
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

    // TODO. 인스턴싱 테스트로 제거 해야함
    FVertex_Instance Temp = {};
    InstanceBuffer = std::make_shared<MVertexBuffer>((uint32)sizeof(FVertex_Instance), 1, &Temp);
    InstanceBuffer2 = std::make_shared<MVertexBuffer>((uint32)sizeof(FVertex_Instance), 1, &Temp);
    
	// 렌더 타겟 추가
	for (int i = 0; i < CastValue<int>(ERenderTarget::Count); ++i)
	{
		FRenderTagetInfo RenderTargetInfo;

		switch (CastValue<ERenderTarget>(i))
		{
		case ERenderTarget::DirectionalShadowDepth:
		{
			RenderTargetInfo.bCube = false;
			RenderTargetInfo.Width = 1024 * 2;
			RenderTargetInfo.Height = 1024 * 2;
            RenderTargetInfo.TextrueNum = CastValue<int>(CascadeDistances.size());
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
            RenderTargetInfo = FRenderTagetInfo::GetDefault();
            RenderTargetInfo.Type = ERenderTargetType::Bool;
        }
		default:
		{
			RenderTargetInfo = FRenderTagetInfo::GetDefault();
		}
		break;
		}

		_renderTargets.emplace_back(std::make_shared<MRenderTarget>(RenderTargetInfo));
	}

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
            ERenderTarget::Depth,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::RimLight
        );
    }

    RenderPasses[EnumToIndex(ERenderPass::Stencil)] = CreateRenderPass<MStencilPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Stencil)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Stencil
        );

        RenderPasses[EnumToIndex(ERenderPass::Stencil)]->SetDefaultShader(TEXT("VS_Stencil.cso"), TEXT("PS_Stencil.cso"));
        RenderPasses[EnumToIndex(ERenderPass::Stencil)]->SetDepthEnable(false);
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
    SpherePID = MPrimitiveComponent::MakePrimitiveID();
    //MakeBuffer(SpherePID, SphereMesh);

    Mesh::MakeCoordinate(CoordinateMesh);
    CoordinatePID = MPrimitiveComponent::MakePrimitiveID();
    //MakeBuffer(CoordinatePID, CoordinateMesh);

    return EnumToIndex(ERenderPass::End) == GetSize(RenderPasses);
}

void MRenderer::Release()
{
    Super::Release();

    _renderTargets.clear();
    RenderPasses.clear();

    DebugRenderTargetMehses.clear();

    //RenderablePrimitiveData.clear();
    PrimitiveDatasPerType.clear();

    PrimitiveComponents.clear();

    PrimitiveDatas.clear();

    InstanceBuffer.reset();
    InstanceBuffer2.reset();
}

void MRenderer::DrawCylinder(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation)
{

}

void MRenderer::DrawSphere(float InRadius, const Vec3& InTranslation, const DirectX::XMVECTORF32& InColor)
{
    FInstancingData RenderData = {};
    RenderData.Scale = { InRadius, InRadius, InRadius };
    RenderData.Translation = InTranslation;

    SphereRenderDatas.push_back(RenderData);
}

void MRenderer::DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale)
{
    Scenes[InWorld->GetID()]->DrawCoordinate(InTranslation, InRotation, InScale);
}
void MRenderer::DrawPrimitive(MWorld* InWorld, std::shared_ptr<StaticMesh>& InMesh, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale, EPrimitiveType InPrimitiveType)
{
    uint32 WorldID = InWorld->GetID();
    getGraphicDevice()->BuildMeshBuffers(0, InMesh);

    uint32 Num = InMesh->GetMeshNum();
    for (uint32 i = 0; i < Num; ++i)
    {
        FPrimitiveData NewPrimitiveData = {};

        NewPrimitiveData.MeshData = &InMesh->GetMeshData(i);
        NewPrimitiveData.Material = InMesh->getMaterial(0);

        FBufferContainer Buffers;
        getGraphicDevice()->GetBuffers(Buffers, InMesh);

        NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[i];
        NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[i];

        Vec4 QuatRot = {};
        XMStoreFloat4(&QuatRot, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&InRotation)));

        NewPrimitiveData.Translation = InTranslation;
        NewPrimitiveData.Rotation = QuatRot;
        NewPrimitiveData.Scale = InScale;

        NewPrimitiveData.PrimitiveType = InPrimitiveType;

        Scenes[WorldID]->DrawPrimitive(NewPrimitiveData);
    }
}

MScene* MRenderer::GetScene(uint32 InWorldID)
{
    auto& Iter = Scenes.find(InWorldID);
    if (Iter != Scenes.end())
    {
        return Iter->second.get();
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
//
//uint32 MRenderer::DrawCapsule(float InRadius, float InHalfHeight)
//{
//	if (CapsuleMeshData.Vertices.empty())
//	{
//        Mesh::MakeCapsule(CapsuleMeshData, InHalfHeight, InRadius);
//	}
//
//	uint32 PrimitiveID = MPrimitiveComponent::MakePrimitiveID();
//	MakeBuffer(PrimitiveID, CapsuleMeshData);
//
//    std::vector<FPrimitiveData> NewPrimitiveDatas;
//    FPrimitiveData NewPrimitivData = {};
//    NewPrimitivData.MeshData = &CapsuleMeshData;
//    NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
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

    std::shared_ptr<StaticMesh> Mesh = nullptr;
    if (auto& MeshComp = InPrimitiveComponent->CastTo<MMeshComponent>())
    {
        Mesh = MeshComp->GetMesh();
    }
    else if (auto& LightComp = InPrimitiveComponent->CastTo<MLightComponent>())
    {
        Mesh = LightComp->GetMesh();
    }

    if (Mesh == nullptr)
    {
        return;
    }

    uint32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();
    PrimitiveComponents[PrimitiveID] = InPrimitiveComponent;
    getGraphicDevice()->BuildMeshBuffers(PrimitiveID, Mesh);

    if (auto& Actor = InPrimitiveComponent->getOwningActor())
    {
        if (auto& Owner = Actor->GetOwner())
        {
            if (auto& World = Owner->CastTo<MWorld>())
            {
                Scenes[World->GetID()]->AddPrimitiveComponent(InPrimitiveComponent, Mesh);
            }
        }
    }
}

//void MRenderer::AddPrimitiveComponentTemp(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent)
//{
//    if (InPrimitiveComponent == nullptr)
//    {
//        return;
//    }
//
//    int32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();
//    if (PrimitiveDatasUnManaged.find(PrimitiveID) == PrimitiveDatasUnManaged.end())
//    {
//        std::vector<FPrimitiveData> PrimitiveDatas;
//        if (InPrimitiveComponent->GetPrimitiveData(PrimitiveDatas))
//        {
//            PrimitiveDatasUnManaged[PrimitiveID] = PrimitiveDatas;
//        }
//    }
//
//    if (auto& Actor = InPrimitiveComponent->getOwningActor())
//    {
//        if (auto& Owner = Actor->GetOwner())
//        {
//            if (auto& World = Owner->CastTo<MWorld>())
//            {
//                Scenes[World->GetID()]->AddPrimitiveComponent(InPrimitiveComponent);
//            }
//        }
//    }
//
//    //RemoveBuffer(PrimitiveID);
//
//    //// UpdateBuffer 함수가 NoRenderPass를 사용하지 않게 구현되어 있어서 구현부를 옮겨옴
//    //for (FPrimitiveData& PrimitiveData : PrimitiveDatasUnManaged[PrimitiveID])
//    //{
//    //    if (PrimitiveData.MeshData == nullptr)
//    //    {
//    //        continue;
//    //    }
//
//    //    MakeBuffer(PrimitiveID, *PrimitiveData.MeshData);
//    //    PrimitiveData.VertexBuffer = VertexBuffers[PrimitiveID].back();
//    //    PrimitiveData.IndexBuffer = IndexBuffers[PrimitiveID].back();
//    //}
//}

//void MRenderer::GetPrimitiveDataFromComponent(std::shared_ptr<MPrimitiveComponent> InComponent)
//{
//    if (InComponent == nullptr)
//    {
//        return;
//    }
//
//    // 프리미티브 데이터를 추출해 옴
//    int32 PrimitiveID = InComponent->GetPrimitiveID();
//    if (PrimitiveDatas.find(PrimitiveID) == PrimitiveDatas.end())
//    {
//        std::vector<FPrimitiveData> NewPrimitiveDatas;
//        if (InComponent->GetPrimitiveData(NewPrimitiveDatas))
//        {
//            PrimitiveDatas[PrimitiveID] = NewPrimitiveDatas;
//        }
//    }
//
//    // 프리미티브 데이터에 버퍼를 채워줌
//    UpdateBuffer(InComponent);
//
//    if (std::shared_ptr<MMeshComponent> MeshComp = InComponent->CastTo<MMeshComponent>())
//    {
//        MeshComp->GetMeshChangedDelegate().Add(this, &MRenderer::UpdateBuffer);
//    }
//
//    for (uint32 i=0; i<GetSize(PrimitiveDatas[PrimitiveID]); ++i)
//    {
//        FPrimitiveData& PrimitiveData = PrimitiveDatas[PrimitiveID][i];
//        PrimitiveDatasPerType[PrimitiveData.PrimitiveType].push_back(PrimitiveData);
//    }
//}

//void MRenderer::MakeBuffer(uint32 InPrimitiveID, const FMeshData& InMeshData)
//{
//    // 그래픽 디바이스 쪽에 옮겨야 할 듯?
//    uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
//    uint32 VertexNum = GetSize(InMeshData.Vertices);
//    std::shared_ptr<MVertexBuffer> VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, InMeshData.Vertices.data());
//    VertexBuffers[InPrimitiveID].push_back(VertexBuffer);
//
//    uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
//    uint32 IndexNum = GetSize(InMeshData.Indices);
//    std::shared_ptr<MIndexBuffer> IndexBuffer = IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, InMeshData.Indices.data()) : nullptr;
//    IndexBuffers[InPrimitiveID].push_back(IndexBuffer);
//}
//
//void MRenderer::MakeBuffer(FBuffers& OutBuffers, const FMeshData& InMeshData)
//{
//    uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
//    uint32 VertexNum = GetSize(InMeshData.Vertices);
//    OutBuffers.VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, InMeshData.Vertices.data());
//
//    uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
//    uint32 IndexNum = GetSize(InMeshData.Indices);
//    OutBuffers.IndexBuffer = IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, InMeshData.Indices.data()) : nullptr;
//}

//void MRenderer::RemoveBuffer(uint32 InPrimitiveID, int32 InIndex)
//{
//    auto& Iter = VertexBuffers.find(InPrimitiveID);
//
//    if (Iter == VertexBuffers.end())
//    {
//        return;
//    }
//
//    if (InIndex == -1)
//    {
//        VertexBuffers.erase(InPrimitiveID);
//        IndexBuffers.erase(InPrimitiveID);
//    }
//    else
//    {
//        if (GetSize(Iter->second) > static_cast<uint32>(InIndex))
//        {
//            Iter->second[InIndex] = nullptr;
//        }
//    }
//}

//void MRenderer::UpdateBuffer(std::shared_ptr<MPrimitiveComponent> InComponent)
//{
//    if (InComponent == nullptr)
//    {
//        return;
//    }
//
//    std::shared_ptr<StaticMesh> Mesh = nullptr;
//    if (auto& MeshComp = InComponent->CastTo<MMeshComponent>())
//    {
//        Mesh = MeshComp->GetMesh();
//    }
//    else if (auto& LightComp = InComponent->CastTo<MLightComponent>())
//    {
//        Mesh = LightComp->GetMesh();
//    }
//
//    if (Mesh == nullptr)
//    {
//        return;
//    }
//
//    uint32 PrimitiveID = InComponent->GetPrimitiveID();
//    uint32 Num = GetSize(PrimitiveDatas[PrimitiveID]);
//
//    if (Num == 0)
//    {
//        // PrimitiveData의 Buffer를 채우는 함수인데, PrimitiveData가 없으면 안됨
//        return;
//    }
//
//    const std::wstring& AssetPath = Mesh->GetAssetPath();
//    auto& Iter = SharedBuffers.find(AssetPath);
//    bool bCreateBuffer = Iter == SharedBuffers.end();
//
//    for (uint32 i = 0; i < Num; ++i)
//    {
//        FPrimitiveData& PrimitiveData = PrimitiveDatas[PrimitiveID][i];
//        if (PrimitiveData.MeshData == nullptr)
//        {
//            continue;
//        }
//
//        const FMeshData& MeshData = *PrimitiveData.MeshData;
//
//        if (bCreateBuffer)
//        {
//            FBuffers NewSharedBuffers = {};
//            MakeBuffer(NewSharedBuffers, MeshData);
//            SharedBuffers[AssetPath].AddBuffers(NewSharedBuffers);
//        }
//
//        PrimitiveData.VertexBuffer = SharedBuffers[AssetPath].VertexBuffers[i];
//        PrimitiveData.IndexBuffer = SharedBuffers[AssetPath].IndexBuffers[i];
//
//        // 클로딩 등으로 전용 버퍼가 필요한 경우
//        if (Mesh->IsClothigMesh(i))
//        {
//            FBuffers NewPrivateBuffers = {};
//            MakeBuffer(NewPrivateBuffers, MeshData);
//            PrimitiveData.VertexBuffer = NewPrivateBuffers.VertexBuffer;
//            PrimitiveData.IndexBuffer = NewPrivateBuffers.IndexBuffer;
//
//            PrivateBuffers[PrimitiveID].AddBuffers(NewPrivateBuffers);
//        }
//    }
//
//    //// 공유하는 버퍼를 채움
//    //uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
//    //uint32 VertexNum = GetSize(MeshData.Vertices);
//    //std::shared_ptr<MVertexBuffer> VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, MeshData.Vertices.data());
//
//    //uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
//    //uint32 IndexNum = GetSize(MeshData.Indices);
//    //std::shared_ptr<MIndexBuffer> IndexBuffer = IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, MeshData.Indices.data()) : nullptr;
//
//    //SharedBuffers.VertexBuffers[_DynamicMesh->GetAssetPath()].push_back(VertexBuffer);
//    //SharedBuffers.IndexBuffers[_DynamicMesh->GetAssetPath()].push_back(IndexBuffer);
//}

//void MRenderer::UpdateBufferInternal(uint32 InPrimitiveID)
//{
//    for (FPrimitiveData& PrimitiveData : PrimitiveDatas[InPrimitiveID])
//    {
//        if (PrimitiveData.MeshData == nullptr)
//        {
//            continue;
//        }
//
//        MakeBuffer(InPrimitiveID, *PrimitiveData.MeshData);
//        PrimitiveData.VertexBuffer = VertexBuffers[InPrimitiveID].back();
//        PrimitiveData.IndexBuffer = IndexBuffers[InPrimitiveID].back();
//    }
//}

//const std::vector<FPrimitiveData>& MRenderer::GetPrimitiveDatas(uint32 InPrimitiveID)
//{
//    {
//        auto& Iter = PrimitiveDatas.find(InPrimitiveID);
//        if (Iter != PrimitiveDatas.end())
//        {
//            return PrimitiveDatas[InPrimitiveID];
//        }
//    }
//    {
//        auto& Iter = PrimitiveDatasUnManaged.find(InPrimitiveID);
//        if (Iter != PrimitiveDatasUnManaged.end())
//        {
//            return PrimitiveDatasUnManaged[InPrimitiveID];
//        }
//    }
//
//
//    return PrimitiveDatas[-1];
//}


//void MRenderer::UpdatePrimitiveTransform(uint32 InPrimitiveID, const Vec3& InTranslation, const Vec4& InRotation, const Vec3& InScale)
//{
//    auto& Iter = PrimitiveDatas.find(InPrimitiveID);
//    if (Iter == PrimitiveDatas.end())
//    {
//        return;
//    }
//
//    for (auto& PrimitiveData : Iter->second)
//    {
//        PrimitiveData.Scale = InScale;
//        PrimitiveData.Rotation = InRotation;
//        PrimitiveData.Translation = InTranslation;
//    }
//}

//void MRenderer::UpdatePrimitiveVertexPos(uint32 InPrimitiveID, const std::vector<Vertex>& InVertices)
//{
//    FMeshData NewMeshData = {};
//    NewMeshData.Vertices.reserve(InVertices.size());
//    for (const Vertex& TempVertex : InVertices)
//    {
//		NewMeshData.Vertices.push_back(TempVertex);
//    }
//
//    auto& Iter = VertexBuffers.find(InPrimitiveID);
//    if (Iter == VertexBuffers.end())
//    {
//        return;
//    }
//
//    if (Iter->second.empty())
//    {
//        return;
//    }
//
//    auto& VertexBuffer = Iter->second.front();
//    VertexBuffer->Update(NewMeshData.Vertices.data());
//}

//std::shared_ptr<MVertexBuffer> MRenderer::GetVertexBuffer(uint32 InId, uint32 InOffset)
//{
//    auto& Buffers = VertexBuffers[InId];
//    if (Buffers.empty() == false)
//    {
//        return Buffers[InOffset];
//    }
//
//    return nullptr;
//}
//
//std::shared_ptr<MIndexBuffer> MRenderer::GetIndexBuffer(uint32 InId, uint32 InOffset)
//{
//    auto& Buffers = IndexBuffers[InId];
//    if (Buffers.empty() == false)
//    {
//        return IndexBuffers[InId][InOffset];
//    }
//
//    return nullptr;
//}

const std::vector<FPrimitiveData>& MRenderer::GetPrimitives(EPrimitiveType InPrimitiveType)
{
    return Scenes[CurrentSceneID]->GetPrimitives(InPrimitiveType);
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

    // 씬 그리기
    //for (auto& [Temp, Scene] : Scenes)
    //{
    //    SceneID = Temp;
    //    RenderScene(Scene);
    //}
    
    /*
    std::vector<FPrimitiveData> PostRenderPrimitiveDatas;

    // 축 그리기
    PrimitiveDatas[CoordinatePID].clear();
    if (CoordinateRenderDatas.empty() == false)
    {
        std::vector<FVertex_Instance> InstanceDatas;
        for (auto& CoordinateRenderData : CoordinateRenderDatas)
        {
            FVertex_Instance NewInstance = {};
            XMMATRIX XMWorldMat = XMMatrixScalingFromVector(XMLoadFloat3(&CoordinateRenderData.Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&CoordinateRenderData.Quaternion)) * XMMatrixTranslationFromVector(XMLoadFloat3(&CoordinateRenderData.Translation));
            XMStoreFloat4x4(&NewInstance.WorldMatrix, XMWorldMat);
            InstanceDatas.push_back(NewInstance);
        }

        std::vector<FPrimitiveData> PrimitiveDatas;
        FPrimitiveData NewPrimitivData = {};
        NewPrimitivData.MeshData = &CoordinateMesh;
        NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
        NewPrimitivData.VertexBuffer = VertexBuffers[CoordinatePID][0];
        NewPrimitivData.IndexBuffer = IndexBuffers[CoordinatePID][0];
        NewPrimitivData.InstanceBuffer = InstanceBuffer2;
        NewPrimitivData.InstanceNum = GetSize(CoordinateRenderDatas);

        InstanceBuffer2->Update(InstanceDatas.data(), NewPrimitivData.InstanceNum);
        PrimitiveDatas.push_back(NewPrimitivData);

        PostRenderPrimitiveDatas.insert(PostRenderPrimitiveDatas.end(), PrimitiveDatas.begin(), PrimitiveDatas.end());
    }
    
    .clear();

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
    std::unique_ptr<MScene> NewScene = std::make_unique<MScene>();
    NewScene->SetWorld(InWorldRenderInfo.SrcWorld);
    Scenes[InWorldRenderInfo.SrcWorld->GetID()] = std::move(NewScene);
}

void MRenderer::RenderWorld(const std::shared_ptr<MWorld>& InWorld)
{
    if (InWorld == nullptr)
    {
        return;
    }

    CurrentSceneID = InWorld->GetID();
    auto& Scene = Scenes[CurrentSceneID];

    // Cascade Shadow
    auto& DirectionalLightComponents = Scene->GetPrimitives(EPrimitiveType::DirectionalLight);
    if (DirectionalLightComponents.empty() == false)
    {
        const std::shared_ptr<MLightComponent>& LightComponent = DirectionalLightComponents[0].PrimitiveComponent.lock()->CastTo<MLightComponent>();

        float tanHalfVertical = tanf(XMConvertToRadians(g_pSetting->getFov() / 2.f));
        float tanHalfHorizen = tanHalfVertical * g_pSetting->getAspectRatio();

        XMMATRIX XMCameraWorldMat = XMLoadFloat4x4(&Scene->GetWorld()->getMainCamera()->getInvesrViewMatrix());
        XMVECTOR LightDirection = XMVector3Normalize(XMLoadFloat3(&LightComponent->GetDirection()));
        XMVECTOR UpVector = XMLoadFloat3(&VEC3UP);
        if (fabs(XMVectorGetX(XMVector3Dot(UpVector, LightDirection))) > 0.999f)
        {
            UpVector = XMVectorSet(1.f, 0, 0, 0);
        }

        for (int cascadeIndex = 0; cascadeIndex < CastValue<int>(EFrustumCascade::Far); ++cascadeIndex)
        {
            float Depth = CascadeDistances[cascadeIndex];
            float NextDepth = CascadeDistances[cascadeIndex + 1];
            float XNear = CascadeDistances[cascadeIndex] * tanHalfHorizen;
            float XFar = CascadeDistances[cascadeIndex + 1] * tanHalfHorizen;
            float YNear = CascadeDistances[cascadeIndex] * tanHalfVertical;
            float YFar = CascadeDistances[cascadeIndex + 1] * tanHalfVertical;
            float DepthCenter = (NextDepth + Depth) / 2.f;

            std::vector<Vec3> FrustumVertices = {
                //near Face
                {XNear,YNear,Depth},
                {-XNear,YNear,Depth},
                {XNear,-YNear,Depth},
                {-XNear,-YNear,Depth},
                //far Face
                {XFar,YFar,NextDepth},
                {-XFar,YFar,NextDepth},
                {XFar,-YFar,NextDepth},
                {-XFar,-YFar,NextDepth}
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

            float Near = std::max(DepthCenter - Radius, 0.1f);
            float Far = Radius * 2.f;
            XMMATRIX OrthoProjMatrix = XMMatrixOrthographicOffCenterLH(-Radius, Radius, -Radius, Radius, 0.f, Far);

            XMStoreFloat4(&CascadeLightPositions[cascadeIndex], Eye);
            XMStoreFloat4x4(&CascadeLightMatrices[cascadeIndex], LightView * OrthoProjMatrix);
            CascadeLightMatrices[cascadeIndex]._42 = round(CascadeLightMatrices[cascadeIndex]._42 * 10.f) / 10.f;
            CascadeLightMatrices[cascadeIndex]._43 = round(CascadeLightMatrices[cascadeIndex]._43 * 10.f) / 10.f;
        }
    }

    UpdateGlobalConstantBuffer();
    UpdateTickConstantBuffer();

    RenderScene(Scene);

    InWorld->GetOnRederedDelegate().Broadcast();
}

std::shared_ptr<MWorld> MRenderer::GetWorld()
{
    return Scenes[CurrentSceneID]->GetWorld();
}

void MRenderer::RenderScene(std::unique_ptr<MScene>& InScene)
{
    InScene->Begin();

    InScene->MakeSpherePrimitives();
    InScene->MakeCoordinatePrimitives();

    TotalPrimitiveNum = GetSize(PrimitiveComponents);
    FrustumCulling(InScene);

    /*
    uint32 RenderPassNum = EnumToIndex(ERenderPass::Combine);
    for (uint32 PassIndex = 0; PassIndex <= RenderPassNum; ++PassIndex)
    {
        if (std::shared_ptr<MRenderPass>& CurrentRenderPass = RenderPasses[PassIndex])
        {
#if RenderPassPerformanceProfiling == 1
            std::wstring Name = TEXT("Pass") + std::to_wstring(PassIndex) + TEXT(" :");
            PerformanceTimer Temp(Name);
#endif
            CurrentRenderPass->RenderPass(RenderablePrimitiveData);
        }
    }
    */

    const auto& RenderablePrimitiveDatas = InScene->GetRenderablePrimitiveData();

    // 프로그램에서 등록한 렌더 패스의 수를 가져와서
    uint32 RenderPassNum = EnumToIndex(ERenderPass::End);
    for (uint32 PassIndex = 0; PassIndex < RenderPassNum; ++PassIndex)
    {
        if (std::shared_ptr<MRenderPass>& CurrentRenderPass = RenderPasses[PassIndex])
        {
#if RenderPassPerformanceProfiling == 1
            std::wstring Name = TEXT("Pass") + std::to_wstring(PassIndex) + TEXT(" :");
            PerformanceTimer Temp(Name);
#endif
            CurrentRenderPass->RenderPass(RenderablePrimitiveDatas);
        }
    }

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

void MRenderer::UpdateGlobalConstantBuffer()
{
    std::shared_ptr<MConstantBuffer>& GlobalCBuffer = MShader::GetSharedConstantBuffer(EConstantBufferLayer::Global);
    if (GlobalCBuffer == nullptr)
    {
        return;
    }

    Vec4 resolution = { g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 0.f, 0.f };
    GlobalCBuffer->SetData(TEXT("resolution"), &resolution);

    BOOL bLight = TRUE;
    GlobalCBuffer->SetData(TEXT("bLight"), &bLight);

    GlobalCBuffer->Commit();

    ID3D11Buffer* DX_Buffer = GlobalCBuffer->getRaw();
    g_pGraphicDevice->getContext()->VSSetConstantBuffers(0, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(0, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->GSSetConstantBuffers(0, 1, &DX_Buffer);
}

void MRenderer::UpdateTickConstantBuffer()
{
    EConstantBufferLayer Layer = EConstantBufferLayer::Tick;
    uint32 LayerIndex = EnumToIndex(Layer);
    std::shared_ptr<MConstantBuffer>& TickBuffer = MShader::GetSharedConstantBuffer(Layer);

    TickBuffer->SetData(TEXT("cascadeDistance"), CascadeDistances.data());
    TickBuffer->SetData(TEXT("lightPos"), CascadeLightPositions.data());
    TickBuffer->SetData(TEXT("lightViewProjMatrix"), CascadeLightMatrices.data());

    TickBuffer->SetData(TEXT("viewMatrix"), &GetWorld()->getMainCameraViewMatrix());
    TickBuffer->SetData(TEXT("projectionMatrix"), &GetWorld()->getMainCameraProjectioinMatrix());
    TickBuffer->SetData(TEXT("identityMatrix"), &IDENTITYMATRIX);
    TickBuffer->SetData(TEXT("orthographicProjectionMatrix"), &GetWorld()->getMainCameraOrthographicProjectionMatrix());
    TickBuffer->SetData(TEXT("inverseOrthographicProjectionMatrix"), &GetWorld()->getMainCamera()->getInverseOrthographicProjectionMatrix());

    TickBuffer->Commit();

    ID3D11Buffer* DX_Buffer = TickBuffer->getRaw();
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->VSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->GSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
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
}

void MScene::End()
{
    CachedTemporalPrimitiveDatas = std::move(TemporalPrimitiveDatas);
}

std::shared_ptr<MWindow> MScene::GetWindow() const
{
    return Window.lock();
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

void MScene::MakeSpherePrimitives()
{

}

void MScene::MakeCoordinatePrimitives()
{
    /*
    uint32 PID = getRenderer()->CoordinatePID;
    FMeshData Mesh = getRenderer()->CoordinateMesh;

    PrimitiveDatas[PID].clear();
    if (CoordinateRenderDatas.empty() == false)
    {
        std::vector<FVertex_Instance> InstanceDatas;
        for (auto& CoordinateRenderData : CoordinateRenderDatas)
        {
            FVertex_Instance NewInstance = {};
            XMMATRIX XMWorldMat = XMMatrixScalingFromVector(XMLoadFloat3(&CoordinateRenderData.Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&CoordinateRenderData.Quaternion)) * XMMatrixTranslationFromVector(XMLoadFloat3(&CoordinateRenderData.Translation));
            XMStoreFloat4x4(&NewInstance.WorldMatrix, XMWorldMat);
            InstanceDatas.push_back(NewInstance);
        }

        FBufferContainer BufferContainer = {};
        getGraphicDevice()->GetBuffers(BufferContainer, );

        std::vector<FPrimitiveData> PrimitiveDatas;
        FPrimitiveData NewPrimitivData = {};
        NewPrimitivData.MeshData = &Mesh;
        NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
        NewPrimitivData.VertexBuffer = BufferContainer.VertexBuffers[0];
        NewPrimitivData.IndexBuffer = BufferContainer.IndexBuffers[0];
        NewPrimitivData.InstanceBuffer = InstanceBuffer2;
        NewPrimitivData.InstanceNum = GetSize(CoordinateRenderDatas);

        InstanceBuffer2->Update(InstanceDatas.data(), NewPrimitivData.InstanceNum);
        PrimitiveDatas.push_back(NewPrimitivData);

        PostRenderPrimitiveDatas.insert(PostRenderPrimitiveDatas.end(), PrimitiveDatas.begin(), PrimitiveDatas.end());
    }
    */
}

void MScene::AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent>& InPrimitiveComponent, std::shared_ptr<StaticMesh>& InMesh)
{
    if (InPrimitiveComponent == nullptr)
    {
        return;
    }

    uint32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();
    PrimitiveComponents[PrimitiveID] = InPrimitiveComponent;

    GetPrimitiveDataFromComponent(InPrimitiveComponent, InMesh);
    InPrimitiveComponent->GetPrimitiveChangedDelegate().Add(this, &MScene::UpdatePrimtiveData);

    if (auto& MeshComp = InPrimitiveComponent->CastTo<MMeshComponent>())
    {
        //MeshComp->GetMeshChangedDelegate().Add(this, &MRenderer::UpdateBuffer);
    }
}

void MScene::GetPrimitiveDataFromComponent(std::shared_ptr<MPrimitiveComponent> InComponent, std::shared_ptr<StaticMesh>& InMesh)
{
    if (InComponent == nullptr)
    {
        return;
    }

    // 프리미티브 데이터를 추출해 옴
    int32 PrimitiveID = InComponent->GetPrimitiveID();
    if (PrimitiveDatas.find(PrimitiveID) == PrimitiveDatas.end())
    {
        std::vector<FPrimitiveData> NewPrimitiveDatas;
        if (InComponent->GetPrimitiveData(NewPrimitiveDatas))
        {
            PrimitiveDatas[PrimitiveID] = NewPrimitiveDatas;
        }
    }

    // 프리미티브 데이터에 버퍼를 채워줌
    UpdateBuffer(PrimitiveID, InMesh);

    for (uint32 i = 0; i < GetSize(PrimitiveDatas[PrimitiveID]); ++i)
    {
        FPrimitiveData& PrimitiveData = PrimitiveDatas[PrimitiveID][i];
        PrimitiveDatasPerType[PrimitiveData.PrimitiveType].push_back(PrimitiveData);
    }
}

void MScene::UpdateBuffer(uint32 InPID, std::shared_ptr<StaticMesh>& InMesh)
{
    uint32 PrimitiveDataNum = GetSize(PrimitiveDatas[InPID]);

    if (PrimitiveDataNum == 0)
    {
        // PrimitiveData의 Buffer를 채우는 함수인데, PrimitiveData가 없으면 안됨
        return;
    }

    FBufferContainer Buffers = {};
    getGraphicDevice()->GetBuffers(Buffers, InMesh);

    for (uint32 i = 0; i < PrimitiveDataNum; ++i)
    {
        FPrimitiveData& PrimitiveData = PrimitiveDatas[InPID][i];

        PrimitiveData.VertexBuffer = Buffers.VertexBuffers[i];
        PrimitiveData.IndexBuffer = Buffers.IndexBuffers[i];
    }
}

void MScene::UpdatePrimtiveData(std::shared_ptr<MPrimitiveComponent> InComponent)
{
    auto& Iter = PrimitiveDatas.find(InComponent->GetPrimitiveID());
    if (Iter != PrimitiveDatas.end())
    {
        PrimitiveDatas.erase(InComponent->GetPrimitiveID());
    }

    std::shared_ptr<StaticMesh> Mesh = nullptr;
    if (auto& MeshComp = InComponent->CastTo<MMeshComponent>())
    {
        Mesh = MeshComp->GetMesh();
    }
    else if (auto& LightComp = InComponent->CastTo<MLightComponent>())
    {
        Mesh = LightComp->GetMesh();
    }

    if (Mesh == nullptr)
    {
        return;
    }

    getGraphicDevice()->BuildMeshBuffers(-1, Mesh);

    GetPrimitiveDataFromComponent(InComponent, Mesh);
}

void MScene::AddPrimitiveDatas(uint32 InPID, std::vector<FPrimitiveData> InPrimitiveDatas)
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

void MScene::DrawCoordinate(const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale)
{
    FInstancingData RenderData = {};
    RenderData.Scale = InScale;
    RenderData.Translation = InTranslation;
    XMStoreFloat4(&RenderData.Quaternion, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&InRotation)));

    CoordinateRenderDatas.push_back(RenderData);
}

void MScene::DrawPrimitive(const FPrimitiveData& InPrimitiveData)
{
    TemporalPrimitiveDatas.push_back(InPrimitiveData);
}

const std::vector<FPrimitiveData>& MScene::GetTemporalPrimitiveDatas() const
{
    return CachedTemporalPrimitiveDatas;
}

void MScene::Func()
{
    const std::shared_ptr<MCamera>& Camera = GetWorld()->getMainCamera();

    XMMATRIX ViewProj = XMMatrixMultiply(XMLoadFloat4x4(&Camera->getViewMatrix()), XMLoadFloat4x4(&Camera->getPerspectiveProjectionMatrix()));
    XMStoreFloat4x4(&ViewPerspectiveProjMatrix, ViewProj);

    XMMATRIX XMOrthoViewProject = XMLoadFloat4x4(&Camera->getViewMatrix()) * XMLoadFloat4x4(&Camera->getOrthographicProjectionMatrix());
    XMStoreFloat4x4(&ViewOrthogonalProjMatrix, XMOrthoViewProject);
}