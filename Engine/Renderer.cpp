#include "Renderer.h"

#include "MoonEngine.h"

#include "MapUtility.h"

// DirectXTK
#include "DirectXTK/SpriteFont.h"

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

// Framework
#include "World.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "MeshComponent.h"
#include "LightComponent.h"
#include "StaticMeshComponent.h"

#include "Texture.h"

#include "Camera.h"

#include "Core/ResourceManager.h"

#include "Utility/PerformanceTimer.h"

#undef max
#undef min

using namespace DirectX;

#define MinimalRendering 0

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
	RenderPasses.resize(CastValue<size_t>(ERenderPass::Count), nullptr);

    GetLevelChangedDelegate().Add([&]() {
        RenderablePrimitiveData.clear();
        PrimitiveDatasPerType.clear();

        PrimitiveComponents.clear();
        PrimitiveDatasRenderPass.clear();

        if (GizmoMeshComp)
        {
            AddPrimitiveComponentTemp(GizmoMeshComp);
        }

        for (auto& RenderTargetFSQ : DebugRenderTargetMehses)
        {
            AddPrimitiveComponentTemp(RenderTargetFSQ.second);
        }

        MakeBuffer(SpherePID, SphereMesh);
    });
}

MRenderer::~MRenderer() noexcept
{
    Release();
}

bool MRenderer::Initialize()
{
    Super::Initialize();

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
            RenderTargetInfo.Type = ERenderTargetType::Depth;
            RenderTargetInfo.TextrueNum *= MaxPointLightNum;
        }
		break;
        //case ERenderTarget::DepthPre:
        //{
        //    RenderTargetInfo = FRenderTagetInfo::GetDefault( );
        //    RenderTargetInfo.Type = ERenderTargetType::Depth;
        //}
        break;
        case ERenderTarget::Depth:
		default:
		{
			RenderTargetInfo = FRenderTagetInfo::GetDefault();
		}
		break;
		}

		_renderTargets.emplace_back(std::make_shared<RenderTarget>(RenderTargetInfo));
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

    //RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)] = CreateRenderPass<PointShadowDepthPass>();
    //{
    //    RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->BindRenderTargets(_renderTargets,
    //        ERenderTarget::PointShadowDepth
    //    );

    //    RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->SetDefaultShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPointPS.cso"), TEXT("ShadowDepthPointGS.cso"));
    //    RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->Color = EngineColors::White;
    //}
#endif

    RenderPasses[EnumToIndex(ERenderPass::Geometry)] = CreateRenderPass<GeometryPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::Depth,
            ERenderTarget::Normal,
            ERenderTarget::Specular
        );

        RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindResourceViews(_renderTargets,
            ERenderTarget::DirectionalShadowDepth,
            ERenderTarget::PointShadowDepth
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
            ERenderTarget::DirectionalShadowDepth
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

    RenderPasses[EnumToIndex(ERenderPass::EditorGizmo)] = CreateRenderPass<MEditorPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::EditorGizmo)]->SetDepthEnable(false);
        RenderPasses[EnumToIndex(ERenderPass::EditorGizmo)]->SetDefaultShader(TEXT("VS_Collision.cso"), TEXT("PS_Collision.cso"));
    }

    GizmoMeshComp = std::make_shared<StaticMeshComponent>();
    GizmoMeshComp->SetPhysics(false);
    GizmoMeshComp->SetMesh(TEXT("Base/axis.fbx"));
    GizmoMeshComp->setScale(0.001f, 0.001f, 0.001f);
    GizmoMeshComp->SceneComponent::Update(0.f);
    for (auto& Material : GizmoMeshComp->GetMesh()->getMaterials())
    {
        Material->setShader(TEXT("VS_VertexColorOut.cso"), TEXT("PS_VertexColorOut.cso"));
    }
    UpdateBuffer(GizmoMeshComp);

    //addRenderTargetForDebug(ERenderTarget::DepthPre);
    addRenderTargetForDebug(ERenderTarget::Diffuse);
    addRenderTargetForDebug(ERenderTarget::Depth);
    addRenderTargetForDebug(ERenderTarget::Normal);
    addRenderTargetForDebug(ERenderTarget::Specular);
    addRenderTargetForDebug(ERenderTarget::LightDiffuse);
    addRenderTargetForDebug(ERenderTarget::LightSpecular);
    //addRenderTargetForDebug(ERenderTarget::DirectionalShadowDepth);
    //addRenderTargetForDebug(ERenderTarget::PointShadowDepth);
    addRenderTargetForDebug(ERenderTarget::Outline);
    addRenderTargetForDebug(ERenderTarget::Stencil);
    addRenderTargetForDebug(ERenderTarget::Collision);

    Mesh::MakeSphere(SphereMesh, 16);
    SpherePID = MPrimitiveComponent::MakePrimitiveID();
    MakeBuffer(SpherePID, SphereMesh);

    Mesh::MakeCoordinate(CoordinateMesh);
    CoordinatePID = MPrimitiveComponent::MakePrimitiveID();
    MakeBuffer(CoordinatePID, CoordinateMesh);

    return EnumToIndex(ERenderPass::Count) == GetSize(RenderPasses);
}

void MRenderer::Release()
{
    Super::Release();

    _renderTargets.clear();
    RenderPasses.clear();

    // 객체가 삭제되는 것이 아니기에 여기서 직접 해제해줘야 메모리 로그가 안남음
    GizmoMeshComp.reset();

    RenderablePrimitiveData.clear();
    PrimitiveDatasPerType.clear();

    PrimitiveComponents.clear();

    IndexBuffers.clear();
    VertexBuffers.clear();

    PrimitiveDatasRenderPass.clear();
}

void MRenderer::DrawCylinder(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation)
{

}

void MRenderer::DrawSphere(float InRadius, const Vec3& InTranslation)
{
    FInstancingData RenderData = {};
    RenderData.Scale = { InRadius, InRadius, InRadius };
    RenderData.Translation = InTranslation;

    SphereRenderDatas.push_back(RenderData);
}

void MRenderer::DrawCoordinate(const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale)
{
    FInstancingData RenderData = {};
    RenderData.Scale = InScale;
    RenderData.Translation = InTranslation;
    XMStoreFloat4(&RenderData.Quaternion, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&InRotation)));

    CoordinateRenderDatas.push_back(RenderData);
}

uint32 MRenderer::DrawVertices(const FMeshData& InMeshData)
{
	std::vector<FPrimitiveData> PrimitiveDatas;
	FPrimitiveData NewPrimitivData = {};
	NewPrimitivData.MeshData = &InMeshData;
	NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
	uint32 PrimitiveID = MPrimitiveComponent::MakePrimitiveID();
	MakeBuffer(PrimitiveID, InMeshData);
	NewPrimitivData.VertexBuffer = VertexBuffers[PrimitiveID][0];
	NewPrimitivData.IndexBuffer = IndexBuffers[PrimitiveID][0];
	PrimitiveDatas.push_back(NewPrimitivData);
    PrimitiveDatasRenderPass.emplace(PrimitiveID, PrimitiveDatas);

	return PrimitiveID;
}

uint32 MRenderer::DrawLine(const std::vector<Vec3>& InWorldPositions)
{
    FMeshData NewMeshData = {};
    NewMeshData.Vertices.reserve(InWorldPositions.size());
    for (auto& InPosition : InWorldPositions)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = InPosition.x;
        NewVertex.Pos.y = InPosition.y;
        NewVertex.Pos.z = InPosition.z;

        NewMeshData.Vertices.push_back(NewVertex);
        NewMeshData.Indices.push_back(GetSize(NewMeshData.Vertices) - 1);
    }

    std::vector<FPrimitiveData> PrimitiveDatas;
    FPrimitiveData NewPrimitivData = {};
    NewPrimitivData.MeshData = &NewMeshData;
    NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
    uint32 PrimitiveID = MPrimitiveComponent::MakePrimitiveID();
    MakeBuffer(PrimitiveID, NewMeshData);
    NewPrimitivData.VertexBuffer = VertexBuffers[PrimitiveID][0];
    NewPrimitivData.IndexBuffer = IndexBuffers[PrimitiveID][0];
    PrimitiveDatas.push_back(NewPrimitivData);
    PrimitiveDatasRenderPass.emplace(PrimitiveID, PrimitiveDatas);

    return PrimitiveID;
}

uint32 MRenderer::DrawCapsule(float InRadius, float InHalfHeight)
{
	if (CapsuleMeshData.Vertices.empty())
	{
        Mesh::MakeCapsule(CapsuleMeshData, InHalfHeight, InRadius);
	}

	uint32 PrimitiveID = MPrimitiveComponent::MakePrimitiveID();
	MakeBuffer(PrimitiveID, CapsuleMeshData);

    std::vector<FPrimitiveData> PrimitiveDatas;
    FPrimitiveData NewPrimitivData = {};
    NewPrimitivData.MeshData = &CapsuleMeshData;
    NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
    NewPrimitivData.VertexBuffer = VertexBuffers[PrimitiveID][0];
    NewPrimitivData.IndexBuffer = IndexBuffers[PrimitiveID][0];
    PrimitiveDatas.push_back(NewPrimitivData);
    PrimitiveDatasRenderPass.emplace(PrimitiveID, PrimitiveDatas);

    return PrimitiveID;
}

void MRenderer::AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent)
{
	if (InPrimitiveComponent == nullptr)
	{
		return;
	}

    uint32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();
    PrimitiveComponents[PrimitiveID] = InPrimitiveComponent;

    GetPrimitiveDataFromComponent(InPrimitiveComponent);
    InPrimitiveComponent->GetPrimitiveChangedDelegate().Add(this, &MRenderer::GetPrimitiveDataFromComponent);
}

void MRenderer::AddPrimitiveComponentTemp(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent)
{
    // 일반적은 용도는 아님.
    // AddPrimitiveComponent, GetPrimitiveDataFromComponent 기능들을 혼합해 구현됨

    if (InPrimitiveComponent == nullptr)
    {
        return;
    }

    int32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();
    if (PrimitiveDatasNoRenderPass.find(PrimitiveID) == PrimitiveDatasNoRenderPass.end())
    {
        std::vector<FPrimitiveData> PrimitiveDatas;
        if (InPrimitiveComponent->GetPrimitiveData(PrimitiveDatas))
        {
            PrimitiveDatasNoRenderPass[PrimitiveID] = PrimitiveDatas;
        }
    }

    RemoveBuffer(PrimitiveID);

    // UpdateBuffer 함수가 NoRenderPass를 사용하지 않게 구현되어 있어서 구현부를 옮겨옴
    for (FPrimitiveData& PrimitiveData : PrimitiveDatasNoRenderPass[PrimitiveID])
    {
        if (PrimitiveData.MeshData == nullptr)
        {
            continue;
        }

        MakeBuffer(PrimitiveID, *PrimitiveData.MeshData);
        PrimitiveData.VertexBuffer = VertexBuffers[PrimitiveID].back();
        PrimitiveData.IndexBuffer = IndexBuffers[PrimitiveID].back();
    }
}

void MRenderer::GetPrimitiveDataFromComponent(std::shared_ptr<MPrimitiveComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    int32 PrimitiveID = InComponent->GetPrimitiveID();
    if (PrimitiveDatasRenderPass.find(PrimitiveID) == PrimitiveDatasRenderPass.end())
    {
        std::vector<FPrimitiveData> PrimitiveDatas;
        if (InComponent->GetPrimitiveData(PrimitiveDatas))
        {
            PrimitiveDatasRenderPass[PrimitiveID] = PrimitiveDatas;
        }
    }

    UpdateBuffer(InComponent);

    if (std::shared_ptr<MMeshComponent> MeshComp = InComponent->CastTo<MMeshComponent>())
    {
        MeshComp->GetMeshChangedDelegate().Add(this, &MRenderer::UpdateBuffer);
    }

    for (uint32 i=0; i<GetSize(PrimitiveDatasRenderPass[PrimitiveID]); ++i)
    {
        FPrimitiveData& PrimitiveData = PrimitiveDatasRenderPass[PrimitiveID][i];
        PrimitiveDatasPerType[PrimitiveData.PrimitiveType].push_back(PrimitiveData);
    }
}

void MRenderer::MakeBuffer(uint32 InPrimitiveID, const FMeshData& InMeshData)
{
    uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
    uint32 VertexNum = GetSize(InMeshData.Vertices);
    std::shared_ptr<MVertexBuffer> VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, InMeshData.Vertices.data());
    VertexBuffers[InPrimitiveID].push_back(VertexBuffer);

    uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
    uint32 IndexNum = GetSize(InMeshData.Indices);
    std::shared_ptr<MIndexBuffer> IndexBuffer = IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, InMeshData.Indices.data()) : nullptr;
    IndexBuffers[InPrimitiveID].push_back(IndexBuffer);
}

void MRenderer::RemoveBuffer(uint32 InPrimitiveID, int32 InIndex)
{
    auto& Iter = VertexBuffers.find(InPrimitiveID);

    if (Iter == VertexBuffers.end())
    {
        return;
    }

    if (InIndex == -1)
    {
        VertexBuffers.erase(InPrimitiveID);
        IndexBuffers.erase(InPrimitiveID);
    }
    else
    {
        if (GetSize(Iter->second) > static_cast<uint32>(InIndex))
        {
            Iter->second[InIndex] = nullptr;
        }
    }
}

void MRenderer::UpdateBuffer(std::shared_ptr<MPrimitiveComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    UpdateBufferInternal(InComponent->GetPrimitiveID());
}

void MRenderer::UpdateBufferInternal(uint32 InPrimitiveID)
{
    for (FPrimitiveData& PrimitiveData : PrimitiveDatasRenderPass[InPrimitiveID])
    {
        if (PrimitiveData.MeshData == nullptr)
        {
            continue;
        }

        MakeBuffer(InPrimitiveID, *PrimitiveData.MeshData);
        PrimitiveData.VertexBuffer = VertexBuffers[InPrimitiveID].back();
        PrimitiveData.IndexBuffer = IndexBuffers[InPrimitiveID].back();
    }
}

const std::vector<FPrimitiveData>& MRenderer::GetPrimitiveDatas(uint32 InPrimitiveID)
{
    {
        auto& Iter = PrimitiveDatasRenderPass.find(InPrimitiveID);
        if (Iter != PrimitiveDatasRenderPass.end())
        {
            return PrimitiveDatasRenderPass[InPrimitiveID];
        }
    }
    {
        auto& Iter = PrimitiveDatasNoRenderPass.find(InPrimitiveID);
        if (Iter != PrimitiveDatasNoRenderPass.end())
        {
            return PrimitiveDatasNoRenderPass[InPrimitiveID];
        }
    }


    return PrimitiveDatasRenderPass[-1];
}


void MRenderer::UpdatePrimitiveTransform(uint32 InPrimitiveID, const Vec3& InTranslation, const Vec4& InRotation, const Vec3& InScale)
{
    auto& Iter = PrimitiveDatasRenderPass.find(InPrimitiveID);
    if (Iter == PrimitiveDatasRenderPass.end())
    {
        return;
    }

    for (auto& PrimitiveData : Iter->second)
    {
        PrimitiveData.Scale = InScale;
        PrimitiveData.Rotation = InRotation;
        PrimitiveData.Translation = InTranslation;
    }
}

void MRenderer::UpdatePrimitiveVertexPos(uint32 InPrimitiveID, const std::vector<Vertex>& InVertices)
{
    FMeshData NewMeshData = {};
    NewMeshData.Vertices.reserve(InVertices.size());
    for (const Vertex& TempVertex : InVertices)
    {
		NewMeshData.Vertices.push_back(TempVertex);
    }

    auto& Iter = VertexBuffers.find(InPrimitiveID);
    if (Iter == VertexBuffers.end())
    {
        return;
    }

    if (Iter->second.empty())
    {
        return;
    }

    auto& VertexBuffer = Iter->second.front();
    VertexBuffer->Update(NewMeshData.Vertices.data());
}

std::shared_ptr<MVertexBuffer> MRenderer::GetVertexBuffer(uint32 InId, uint32 InOffset)
{
    auto& Buffers = VertexBuffers[InId];
    if (Buffers.empty() == false)
    {
        return Buffers[InOffset];
    }

    return nullptr;
}

std::shared_ptr<MIndexBuffer> MRenderer::GetIndexBuffer(uint32 InId, uint32 InOffset)
{
    auto& Buffers = IndexBuffers[InId];
    if (Buffers.empty() == false)
    {
        return IndexBuffers[InId][InOffset];
    }

    return nullptr;
}

void MRenderer::addRenderTargetForDebug(ERenderTarget InRenderTarget)
{
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
}

void MRenderer::Render()
{
    // 디렉셔널 라이트 데이터 갱신
    auto DirectionalLightPrimitive = GetPrimitives(EPrimitiveType::DirectionalLight);
    if (DirectionalLightPrimitive.size() > 0)
    {
        const std::shared_ptr<MLightComponent>& InLightComponent = std::static_pointer_cast<MLightComponent>(DirectionalLightPrimitive[0].PrimitiveComponent.lock());

        float tanHalfVertical = tan(XMConvertToRadians(g_pSetting->getFov() / 2.f));
        float tanHalfHorizen = tanHalfVertical * g_pSetting->getAspectRatio();
        XMMATRIX cameraWorldMatrix = XMLoadFloat4x4(&g_World->getMainCamera()->getInvesrViewMatrix());

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

            // CascadeFrustum의 중심 위치를 구함
            XMVECTOR CascadeCenter = XMVectorSet(VEC4ZERO.x, VEC4ZERO.y, VEC4ZERO.z, VEC4ZERO.w);
            for (auto& vertex : FrustumVertices)
            {
                CascadeCenter += XMLoadFloat3(&vertex);
            }
            CascadeCenter /= static_cast<float>(FrustumVertices.size());

            // 중간위치와 가장 먼 점을 기준으로 Radius 설정
            float radius = 0.f;
            for (auto& vertex : FrustumVertices)
            {
                Vec3 Distance;
                XMStoreFloat3(&Distance, XMVector3Length(XMLoadFloat3(&vertex) - CascadeCenter));

                radius = std::max<float>(Distance.x, radius);
            }
            //radius = std::ceil(radius * 2.f) / 2.f;

            XMVECTOR LightDirection = XMVector3Normalize(XMLoadFloat3(&InLightComponent->GetDirection()));
            XMVECTOR CascadeCenterInWorld = XMVector3TransformCoord(CascadeCenter, cameraWorldMatrix);
            XMVECTOR LightPositionInFrustume = CascadeCenterInWorld - (LightDirection * radius);
            XMStoreFloat4(&CascadeLightPositions[cascadeIndex], LightPositionInFrustume);
            CascadeLightPositions[cascadeIndex].w = 1.f;

            float Near = std::max(DepthCenter - radius, 0.1f);
            float Far = radius * 2.f;
            XMMATRIX OrthoProjMatrix = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, Near, Far);

            XMVECTOR UpVector = XMLoadFloat3(&VEC3UP);
            if (fabs(XMVectorGetX(XMVector3Dot(UpVector, LightDirection))) > 0.999f)
            {
                UpVector = XMVectorSet(1.f, 0, 0, 0);
            }
            XMMATRIX LightView = XMMatrixLookAtLH(LightPositionInFrustume, LightPositionInFrustume + LightDirection, UpVector);
            //XMVECTOR projCenter = XMVector3TransformCoord(CascadeCenter, LightView);
            //float worldTexelSize = radius * 2.f / 2048.f;
            //float snapX = roundf(XMVectorGetX(projCenter) / worldTexelSize) * worldTexelSize;
            //float snapY = roundf(XMVectorGetY(projCenter) / worldTexelSize) * worldTexelSize;
            //LightView.r[3].m128_f32[0] += (snapX - XMVectorGetX(projCenter));
            //LightView.r[3].m128_f32[1] += (snapY - XMVectorGetY(projCenter));
            XMMATRIX XMLightViewProj = LightView  * OrthoProjMatrix;
            XMStoreFloat4x4(&CascadeLightMatrices[cascadeIndex], XMLightViewProj);
            CascadeLightMatrices[cascadeIndex]._42 = round(CascadeLightMatrices[cascadeIndex]._42 * 10.f) / 10.f;
            CascadeLightMatrices[cascadeIndex]._43 = round(CascadeLightMatrices[cascadeIndex]._43 * 10.f) / 10.f;
        }
    }

    UpdateGlobalConstantBuffer();
    UpdateTickConstantBuffer();

    // 스피어 그리기
    PrimitiveDatasRenderPass[SpherePID].clear();
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

        std::vector<FPrimitiveData> PrimitiveDatas;
        FPrimitiveData NewPrimitivData = {};
        NewPrimitivData.MeshData = &SphereMesh;
        NewPrimitivData.PrimitiveType = EPrimitiveType::Collision;
        NewPrimitivData.VertexBuffer = VertexBuffers[SpherePID][0];
        NewPrimitivData.IndexBuffer = IndexBuffers[SpherePID][0];
        NewPrimitivData.InstanceBuffer = InstanceBuffer;
        NewPrimitivData.InstanceNum = GetSize(SphereRenderDatas);

        InstanceBuffer->Update(InstanceDatas.data(), NewPrimitivData.InstanceNum);
        PrimitiveDatas.push_back(NewPrimitivData);

        PrimitiveDatasRenderPass[SpherePID].insert(PrimitiveDatasRenderPass[SpherePID].end(), PrimitiveDatas.begin(), PrimitiveDatas.end());
    }
    SphereRenderDatas.clear();

    // 씬 그리기
	RenderScene();
    
    std::vector<FPrimitiveData> PostRenderPrimitiveDatas;

    // 축 그리기
    PrimitiveDatasRenderPass[CoordinatePID].clear();
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
    CoordinateRenderDatas.clear();

    // 기즈모
    if (bGizmo)
    {
        float DistToScale = XMVectorGetX(XMVector3Length(XMLoadFloat3(&g_World->getMainCamera()->GetWorldTranslation()) - XMLoadFloat3(&GizmoPos))) / 10.f;

        GizmoMeshComp->setTranslation(GizmoPos);
        GizmoMeshComp->setScale(0.001f * DistToScale, 0.001f * DistToScale, 0.001f * DistToScale);
        GizmoMeshComp->SceneComponent::Update(0.f);

        const std::vector<FPrimitiveData>& GizmoPrimitives = GetPrimitiveDatas(GizmoMeshComp->GetPrimitiveID());
        PostRenderPrimitiveDatas.insert(PostRenderPrimitiveDatas.end(), GizmoPrimitives.begin(), GizmoPrimitives.end());
    }

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

    RenderPasses[(int)ERenderPass::EditorGizmo]->RenderPass(PostRenderPrimitiveDatas);
}

void MRenderer::RenderScene()
{
	TotalPrimitiveNum = GetSize(PrimitiveComponents);
	FrustumCulling();

	uint32 CombinePass = EnumToIndex(ERenderPass::Combine);
	for (uint32 PassIndex = 0; PassIndex <= CombinePass; ++PassIndex)
	{
        if (std::shared_ptr<MRenderPass>& CurrentRenderPass = RenderPasses[PassIndex])
        {
            std::wstring Name = TEXT("Pass") + std::to_wstring(PassIndex) + TEXT(" :");
            PerformanceTimer Temp(Name);
            CurrentRenderPass->RenderPass(RenderablePrimitiveData);
        }
	}
}

void MRenderer::RenderText()
{

}

void MRenderer::FrustumCulling()
{
    RenderablePrimitiveData.clear();

    const std::shared_ptr<MCamera>& Camera = g_World->getMainCamera();

	XMMATRIX ViewProj = XMMatrixMultiply(XMLoadFloat4x4(&Camera->getViewMatrix()), XMLoadFloat4x4(&g_World->getMainCameraProjectioinMatrix()));
	XMStoreFloat4x4(&ViewPerspectiveProjMatrix, ViewProj);

    XMMATRIX XMOrthoViewProject = XMLoadFloat4x4(&Camera->getViewMatrix()) * XMLoadFloat4x4(&g_World->getMainCameraOrthographicProjectionMatrix());
    XMStoreFloat4x4(&ViewOrthogonalProjMatrix, XMOrthoViewProject);

    // 평면의 방정식 ax + by + cz + d = 0을 구해야 함

    // 절두체 Near 평면
	std::vector<XMVECTOR> Planes(6);
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

    //RenderablePrimitiveComponents.clear();
    TotalPrimitiveNum = 0;
    CulledPrimitiveNum = 0;
    ShownPrimitiveNum = 0;

    for (auto& [Id, PrimitiveDatas] : PrimitiveDatasRenderPass)
    {
        if (PrimitiveDatas.empty())
        {
            continue;
        }

        TotalPrimitiveNum += GetSize(PrimitiveDatas);

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

        RenderablePrimitiveData.insert(RenderablePrimitiveData.end(), PrimitiveDatas.begin(), PrimitiveDatas.end());
        ShownPrimitiveNum += GetSize(PrimitiveDatas);
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
    std::shared_ptr<MConstantBuffer>& TickCBuffer = MShader::GetSharedConstantBuffer(Layer);

    TickCBuffer->SetData(TEXT("cascadeDistance"), CascadeDistances.data());
    TickCBuffer->SetData(TEXT("lightPos"), CascadeLightPositions.data());
    TickCBuffer->SetData(TEXT("lightViewProjMatrix"), CascadeLightMatrices.data());

    TickCBuffer->SetData(TEXT("viewMatrix"), &g_World->getMainCameraViewMatrix());
    TickCBuffer->SetData(TEXT("projectionMatrix"), &g_World->getMainCameraProjectioinMatrix());
    TickCBuffer->SetData(TEXT("identityMatrix"), &IDENTITYMATRIX);
    TickCBuffer->SetData(TEXT("orthographicProjectionMatrix"), &g_World->getMainCameraOrthographicProjectionMatrix());
    TickCBuffer->SetData(TEXT("inverseOrthographicProjectionMatrix"), &g_World->getMainCamera()->getInverseOrthographicProjectionMatrix());

    TickCBuffer->Commit();

    ID3D11Buffer* DX_Buffer = TickCBuffer->getRaw();
    g_pGraphicDevice->getContext()->PSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->VSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
    g_pGraphicDevice->getContext()->GSSetConstantBuffers(LayerIndex, 1, &DX_Buffer);
}