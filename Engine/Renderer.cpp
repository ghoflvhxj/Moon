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
#include "Module/Render/RenderPass/FullScreenQuadPass/FullScreenQuadPass.h"
#include "Module/Render/RenderPass/FullScreenQuadPass/PointLightPass.h"
#include "Module/Render/RenderPass/FullScreenQuadPass/DirectionalLightPass.h"
#include "Module/Render/RenderPass/FullScreenQuadPass/SSAOPass.h"
#include "Module/Render/Scene.h"

#include "Material.h"
#include "Module/Graphic/Shader/Shader.h"

#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"

// Framework
#include "World.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "MeshComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"
#include "Framework/Component/FX/FXComponent.h"

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
constexpr UINT STBufferSlot_Component = 10;
constexpr UINT StructuredBufferSlot = 100;

MRenderer::MRenderer() noexcept
{
	RenderPasses.resize(CastValue<size_t>(ERenderPass::End), nullptr);

    GetLevelChangedDelegate().Add([&]() {
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

    Weights = std::move(MakeGaussianWeights(GaussianRadius, GaussianSigma));
    Buffer = getGraphicDevice()->AddStructuredBuffer(Weights.data(), sizeof(float) * Weights.size(), Weights.size(), sizeof(float), StructuredBufferSlot);

    // BindRenderTargets 컴파일 성공용. 제거해야함
    RenderTargets _renderTargets;

    //RenderPasses[EnumToIndex(ERenderPass::ZPre)] = CreateRenderPass<MDepthPre>();
    //{
    //    RenderPasses[EnumToIndex(ERenderPass::ZPre)]->SetDefaultShader(TEXT("TexAnimVertexShader.cso"), nullptr);
    //    RenderPasses[EnumToIndex(ERenderPass::ZPre)]->ApplyDefaultShaderOnly(true);
    //}

#if MinimalRendering == 0
    RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)] = CreateRenderPass<DirectionalShadowDepthPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->BindRenderTargets(_renderTargets,
            ERenderTarget::DirectionalShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->SetDefaultShader(TEXT("VS_DirectionalLightShadow.cso"), nullptr, TEXT("ShadowDepthGS.cso"));
        RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->Color = EngineColors::White;
    }

    RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)] = CreateRenderPass<PointShadowDepthPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->BindRenderTargets(_renderTargets,
            ERenderTarget::PointShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->SetDefaultShader(TEXT("VS_PointLightShadow.cso"), TEXT("PS_PointLightShadow.cso"), TEXT("GS_PointShadowDepth.cso"));
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->Color = EngineColors::White;
    }
#endif

    RenderPasses[EnumToIndex(ERenderPass::Geometry)] = CreateRenderPass<GeometryPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::Emissive,
            ERenderTarget::RimLight
        );
    }

    RenderPasses[EnumToIndex(ERenderPass::FX)] = CreateRenderPass<MFXPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::FX)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::Emissive,
            ERenderTarget::RimLight
        );

        RenderPasses[EnumToIndex(ERenderPass::FX)]->SetClearTargets(false);
        RenderPasses[EnumToIndex(ERenderPass::FX)]->SetDefaultShader(EShaderType::Compute, TEXT("CS_Particle.cso"));
    }

    RenderPasses[EnumToIndex(ERenderPass::SSAO)] = CreateRenderPass<MSSAOPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SSAO)]->BindRenderTargets(_renderTargets,
            ERenderTarget::SSAO
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAO)]->BindResourceViews(_renderTargets
            , ERenderTarget::Normal
            , ERenderTarget::Depth
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAO)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_AlchemySSAO.cso"));
        RenderPasses[EnumToIndex(ERenderPass::SSAO)]->ApplyDefaultShaderOnly(true);
    }

    RenderPasses[EnumToIndex(ERenderPass::SSAODownSample)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SSAODownSample)]->BindRenderTargets(_renderTargets,
            ERenderTarget::SSAODownSample
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAODownSample)]->BindResourceViews(_renderTargets
            , ERenderTarget::SSAO
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAODownSample)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_BoxBlur.cso"));
        RenderPasses[EnumToIndex(ERenderPass::SSAODownSample)]->ApplyDefaultShaderOnly(true);
        RenderPasses[EnumToIndex(ERenderPass::SSAODownSample)]->bLikeMaterial = true;
    }

    RenderPasses[EnumToIndex(ERenderPass::SSAOBlurRow)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurRow)]->BindRenderTargets(_renderTargets,
            ERenderTarget::SSAOBlurRow
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurRow)]->BindResourceViews(_renderTargets
            , ERenderTarget::SSAODownSample
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurRow)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_BoxBlur.cso"));
        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurRow)]->ApplyDefaultShaderOnly(true);
        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurRow)]->bLikeMaterial = true;
    }

    RenderPasses[EnumToIndex(ERenderPass::SSAOBlurCol)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurCol)]->BindRenderTargets(_renderTargets,
            ERenderTarget::SSAOBlurCol
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurCol)]->BindResourceViews(_renderTargets
            , ERenderTarget::SSAOBlurRow
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurCol)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_BoxBlur.cso"));
        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurCol)]->ApplyDefaultShaderOnly(true);
        RenderPasses[EnumToIndex(ERenderPass::SSAOBlurCol)]->bLikeMaterial = true;
    }

    RenderPasses[EnumToIndex(ERenderPass::SSAOUpSample)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SSAOUpSample)]->BindRenderTargets(_renderTargets,
            ERenderTarget::SSAOUpSample
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAOUpSample)]->BindResourceViews(_renderTargets
            , ERenderTarget::SSAOBlurCol
        );

        RenderPasses[EnumToIndex(ERenderPass::SSAOUpSample)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_DownSample.cso"));
        RenderPasses[EnumToIndex(ERenderPass::SSAOUpSample)]->ApplyDefaultShaderOnly(true);
        RenderPasses[EnumToIndex(ERenderPass::SSAOUpSample)]->bLikeMaterial = true;
    }


#if MinimalRendering == 0
    RenderPasses[EnumToIndex(ERenderPass::EmissiveDownSample)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::EmissiveDownSample)]->BindRenderTargets(_renderTargets,
            ERenderTarget::EmissiveDownSampled
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveDownSample)]->BindResourceViews(_renderTargets,
            ERenderTarget::Emissive
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveDownSample)]->bLikeMaterial = true;
        RenderPasses[EnumToIndex(ERenderPass::EmissiveDownSample)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_DownSample.cso"));
    }

    RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurRow)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurRow)]->BindRenderTargets(_renderTargets,
            ERenderTarget::EmissiveBlurRow
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurRow)]->BindResourceViews(_renderTargets,
            ERenderTarget::EmissiveDownSampled
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurRow)]->bLikeMaterial = true;
        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurRow)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_GaussianBlurRow.cso"));
        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurRow)]->GetHandlePixelShaderStageDelegate().Add([&](const FPrimitiveData& InPrimitiveData, std::shared_ptr<MShader> InPixelShader) {
            getGraphicDevice()->PSSetSRV(Buffer);
            });
    }

    RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurCol)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurCol)]->BindRenderTargets(_renderTargets,
            ERenderTarget::EmissiveBlurCol
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurCol)]->BindResourceViews(_renderTargets,
            ERenderTarget::EmissiveBlurRow
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurCol)]->bLikeMaterial = true;
        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurCol)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_GaussianBlurCol.cso"));
        RenderPasses[EnumToIndex(ERenderPass::EmissiveBlurCol)]->GetHandlePixelShaderStageDelegate().Add([&](const FPrimitiveData& InPrimitiveData, std::shared_ptr<MShader> InPixelShader) {
            getGraphicDevice()->PSSetSRV(Buffer);
            });
    }

    RenderPasses[EnumToIndex(ERenderPass::EmissiveUpSample)] = CreateRenderPass<MFullScreenQuadPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::EmissiveUpSample)]->BindRenderTargets(_renderTargets,
            ERenderTarget::EmissiveUpSampled
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveUpSample)]->BindResourceViews(_renderTargets,
            ERenderTarget::EmissiveBlurCol
        );

        RenderPasses[EnumToIndex(ERenderPass::EmissiveUpSample)]->bLikeMaterial = true;
        RenderPasses[EnumToIndex(ERenderPass::EmissiveUpSample)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("PS_DownSample.cso"));
    }

    RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)] = CreateRenderPass<DirectionalLightPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)]->BindRenderTargets(_renderTargets,
            ERenderTarget::LightDirectDiffuse,
            ERenderTarget::LightDirectSpecular,
            ERenderTarget::InDirectDiffuse
        );

        RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)]->BindResourceViews(_renderTargets,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::Depth,
            ERenderTarget::DirectionalShadowDepth,
            ERenderTarget::RimLight
        );
    }

    RenderPasses[EnumToIndex(ERenderPass::PointLight)] = CreateRenderPass<PointLightPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->BindRenderTargets(_renderTargets,
            ERenderTarget::LightDirectDiffuse,
            ERenderTarget::LightDirectSpecular,
            ERenderTarget::InDirectDiffuse
        );
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->BindResourceViews(_renderTargets,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::Depth,
            ERenderTarget::PointShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->SetDefaultShader(TEXT("Light.cso"), TEXT("PointLightShader.cso"));
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->SetClearTargets(false);
    }

    RenderPasses[EnumToIndex(ERenderPass::SkyPass)] = CreateRenderPass<SkyPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::SkyPass)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Diffuse,
            ERenderTarget::LightDirectDiffuse
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
            ERenderTarget::Emissive,
            ERenderTarget::LightDirectDiffuse,
            ERenderTarget::LightDirectSpecular,
            ERenderTarget::InDirectDiffuse,
            ERenderTarget::Collision,
            ERenderTarget::SSAOUpSample,
            ERenderTarget::Outline,
            ERenderTarget::EmissiveUpSampled
        );

        RenderPasses[EnumToIndex(ERenderPass::Combine)]->SetDefaultShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));
    }

    Mesh::MakeSphere(SphereMesh, 16);

    Mesh::MakeCoordinate(CoordinateMesh);
    getGraphicDevice()->BuildMeshBuffer(CoordinateKey, CoordinateMesh, 0);

    std::shared_ptr<StaticMesh> PlaneMesh = g_ResourceManager->Load(TEXT("Base/Plane.json"), StaticMesh::GetTypeDescStatic())->CastToShared<StaticMesh>();
    getGraphicDevice()->BuildMeshSharedBuffers(PlaneMesh);

    return EnumToIndex(ERenderPass::End) == GetSize(RenderPasses);
}

void MRenderer::Release()
{
    Super::Release();

    //_renderTargets.clear();
    RenderTargetss.clear();
    RenderPasses.clear();

    DebugRenderTargetData.clear();

    //PrimitiveDatasPerType.clear();

    //PrimitiveDatas.clear();
}

void MRenderer::DrawPrimitive(MWorld* InWorld, const std::shared_ptr<StaticMesh>& InMesh, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale, EPrimitiveType InPrimitiveType)
{
    uint32 WorldID = InWorld->GetID();

    getGraphicDevice()->BuildMeshSharedBuffers(InMesh);
    FMeshBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, InMesh);

    uint32 Num = InMesh->GetMeshNum();
    for (uint32 i = 0; i < Num; ++i)
    {
        FPrimitiveData NewPrimitiveData = {};

        NewPrimitiveData.MeshData = &InMesh->GetMeshData(i);
        NewPrimitiveData.Material = InMesh->getGeometryLinkMaterialIndex().empty() ? InMesh->getMaterial(0) : InMesh->getMaterial(InMesh->getGeometryLinkMaterialIndex()[i]);
        NewPrimitiveData.PrimitiveType = InPrimitiveType;

        NewPrimitiveData.Translation = InTranslation;
        NewPrimitiveData.Rotation = EulerToQuaternion(InRotation);
        NewPrimitiveData.Scale = InScale;

        NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[i];
        NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[i];

        Scenes[WorldID]->AddTemporalPrimitiveData(NewPrimitiveData);

        // 인스턴싱 무조건 사용
        //Scenes[WorldID]->AddTemporalInstanceData(InMesh->GetAssetPath(), NewPrimitiveData, Buffers.InstanceBuffers[i]);

        /*
        if (bInstance)
        {
            Scenes[WorldID]->AddTemporalInstanceData(InMesh->GetAssetPath(), NewPrimitiveData, Buffers.InstanceBuffers[i]);
        }
        else
        {
            Scenes[WorldID]->AddTemporalPrimitiveData(NewPrimitiveData);
        }
        */
    }
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

    FMeshBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, BufferKey);
    NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[0];
    NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[0];

    Scenes[InWorld->GetID()]->AddTemporalInstanceData(BufferKey, NewPrimitiveData, Buffers.InstanceBuffers[0]);
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

    FMeshBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, CoordinateKey);
    NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[0];
    NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[0];

    Scenes[InWorld->GetID()]->AddTemporalInstanceData(CoordinateKey, NewPrimitiveData, Buffers.InstanceBuffers[0]);
}

void MRenderer::DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale)
{
    DrawCoordinate(InWorld, InTranslation, EulerToQuaternion(InRotation), InScale);
}

/*
std::shared_ptr<MRenderTarget> MRenderer::UpDownSampling(std::shared_ptr<MRenderTarget> InRenderTarget, uint32 InWidth, uint32 InHeight)
{
    assert(InRenderTarget);

    std::shared_ptr<MRenderTarget> RenderTarget = std::make_shared<MRenderTarget>();

    FRenderTagetInfo RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
    RenderTargetInfo.Type = ERenderTargetType::Light;
    RenderTarget->initializeTexture(RenderTargetInfo);

    MRenderPass RenderPass;

    RenderTargets Dummy;
    RenderPass.SetRenderTarget(RenderTarget);

    FBufferContainer BufferContainer = {};
    getGraphicDevice()->GetBuffers(BufferContainer, TEXT("Plane"));

    std::shared_ptr<MMaterial> Material = std::make_shared<MMaterial>();
    Material->setShader(TEXT("Deferred.cso"), TEXT("PS_DownSample.cso"));
    Material->setTexture(ETextureType::Diffuse, InRenderTarget->AsTexture());

    FPrimitiveData PrimitiveData = {};
    PrimitiveData.MeshData = &MeshData;
    PrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
    PrimitiveData.Material = Material;
    PrimitiveData.Scale.x = static_cast<float>(1920);
    PrimitiveData.Scale.y = static_cast<float>(1080);
    PrimitiveData.ProjectionType = EProjectionType::Orthograhpic;

    PrimitiveData.VertexBuffer = BufferContainer.VertexBuffers[0];
    PrimitiveData.IndexBuffer = BufferContainer.IndexBuffers[0];

    std::vector<FPrimitiveData> PrimitiveDatas;
    PrimitiveDatas.push_back(PrimitiveData);

    RenderPass.RenderPass(PrimitiveDatas);

    return RenderTarget;
}

std::shared_ptr<MRenderTarget> MRenderer::Blur(std::shared_ptr<MRenderTarget> InRenderTarget)
{
    assert(InRenderTarget);

    std::shared_ptr<MRenderTarget> RenderTarget = std::make_shared<MRenderTarget>();

    FRenderTagetInfo RenderTargetInfo = InRenderTarget->GetRenderTargetInfo();
    RenderTarget->initializeTexture(RenderTargetInfo);

    MRenderPass RenderPass;

    RenderTargets Dummy;
    RenderPass.SetRenderTarget(RenderTarget);

    FBufferContainer BufferContainer = {};
    getGraphicDevice()->GetBuffers(BufferContainer, TEXT("Plane"));

    std::shared_ptr<MMaterial> Material = std::make_shared<MMaterial>();
    Material->setShader(TEXT("Deferred.cso"), TEXT("PS_BoxBlur.cso"));
    Material->setTexture(ETextureType::Diffuse, InRenderTarget->AsTexture());

    FPrimitiveData PrimitiveData = {};
    PrimitiveData.MeshData = &MeshData;
    PrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
    PrimitiveData.Material = Material;
    PrimitiveData.Scale.x = static_cast<float>(1920);
    PrimitiveData.Scale.y = static_cast<float>(1080);
    PrimitiveData.ProjectionType = EProjectionType::Orthograhpic;

    PrimitiveData.VertexBuffer = BufferContainer.VertexBuffers[0];
    PrimitiveData.IndexBuffer = BufferContainer.IndexBuffers[0];

    std::vector<FPrimitiveData> PrimitiveDatas;
    PrimitiveDatas.push_back(PrimitiveData);

    RenderPass.RenderPass(PrimitiveDatas);

    return RenderTarget;
}
*/

std::vector<float> MRenderer::MakeGaussianWeights(int InRadius, float InSigma)
{
    int Size = InRadius * 2 + 1;
    std::vector<float> Weights(Size);
    float Sum = 0.f;

    for (int i = -InRadius; i <= InRadius; ++i)
    {
        float Exponent = -(float)(i * i) / (2.f * InSigma * InSigma);
        float Weight = (1.f / (std::sqrt(2.f * XM_PI) * InSigma)) * std::exp(Exponent);
        Weights[i + InRadius] = Weight;
        Sum += Weight;
    }

    return Weights;
}

void MRenderer::Test(uint32 InWorldID, uint32 InPID, std::shared_ptr<MMesh> InMesh)
{
    for (auto& [SceneID, Scene] : Scenes)
    {
        if (SceneID == InWorldID)
        {
            Scene->UpdateBuffersFromMesh(InPID, InMesh);
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
    assert(InPrimitiveComponent);

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
    bool bInstancing = false;
    if (MMeshComponent* MeshComp = InComponent->CastTo<MMeshComponent>())
    {
        Mesh = MeshComp->GetMesh();
    }
    else if (MFXComponent* FxComp = InComponent->CastTo<MFXComponent>())
    {
        Mesh = FxComp->GetMesh();
        for (auto& PrimitiveData : NewPrimitiveDatas)
        {
            PrimitiveData.InstanceNum = GetSize(FxComp->Particles);
        }
    }

    if (Mesh == nullptr)
    {
        return;
    }

    getGraphicDevice()->BuildMeshBuffersFromComponent(PrimitiveID, Mesh);
    Scene->UpdateBuffersFromMesh(PrimitiveID, Mesh);

    //Scene->UpdateBuffersFromShader(PrimitiveID);
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
            RenderTargetInfo.Type = ERenderTargetType::Diffuse;
        }
        break;
        case ERenderTarget::Normal:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Normal;
        }
        break;
        case ERenderTarget::Depth:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Depth;
        }
        break;
        case ERenderTarget::SSAO:
        case ERenderTarget::SSAOUpSample:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            //RenderTargetInfo.Type = ERenderTargetType::SingleFloat16;
        }
        break;
        case ERenderTarget::SSAODownSample:
        case ERenderTarget::SSAOBlurRow:
        case ERenderTarget::SSAOBlurCol:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth / 4, InHeight / 4);
            //RenderTargetInfo.Type = ERenderTargetType::SingleFloat16;
        }
        break;
        case ERenderTarget::Emissive:
        case ERenderTarget::EmissiveUpSampled:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth, InHeight);
            RenderTargetInfo.Type = ERenderTargetType::Light;
        }
        break;
        case ERenderTarget::EmissiveBlurRow:
        case ERenderTarget::EmissiveBlurCol:
        case ERenderTarget::EmissiveDownSampled:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault(InWidth / 4, InHeight / 4);
            RenderTargetInfo.Type = ERenderTargetType::Light;
        }
        break;
        case ERenderTarget::LightDirectDiffuse:
        case ERenderTarget::LightDirectSpecular:
        case ERenderTarget::InDirectDiffuse:
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
            RenderTargetInfo.TextrueNum = CastValue<int>(ECascade::Count);
            RenderTargetInfo.Type = ERenderTargetType::Depth;
        }
        break;
        case ERenderTarget::PointShadowDepth:
        {
            static constexpr uint32 MaxPointLightNum = 8;
            RenderTargetInfo = FRenderTagetInfo::GetCube();
            RenderTargetInfo.Width = 512;
            RenderTargetInfo.Height = 512;
            RenderTargetInfo.TextrueNum = MaxPointLightNum;
            RenderTargetInfo.Type = ERenderTargetType::LinearDepth;
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
#ifdef _DEBUG
    if (DebugRenderTargetData.find(InRenderTarget) == DebugRenderTargetData.end())
    {
        std::shared_ptr<MMaterial> BaseMat = nullptr;
        std::shared_ptr<MMaterial> NewMat = nullptr;

        if (InRenderTarget == ERenderTarget::DirectionalShadowDepth)
        {
            g_ResourceManager->Load(TEXT("Base/RenderTarget_Depth.json"), BaseMat);
            NewMat = DuplicateObject(BaseMat)->CastToShared<MMaterial>();
            NewMat->Test();
        }
        else
        {
            g_ResourceManager->Load(TEXT("Base/RenderTarget.json"), BaseMat);
            NewMat = DuplicateObject(BaseMat)->CastToShared<MMaterial>();
        }

        FRenderTargetDebugData NewData = {};
        NewData.Material = NewMat;
        NewData.Index = CastValue<uint32>(DebugRenderTargetData.size());

        DebugRenderTargetData.emplace(InRenderTarget, NewData);
    }

    DebugRenderTargetData[InRenderTarget].Material->setTexture(ETextureType::Diffuse, GetRenderTarget(InRenderTarget)->AsTexture());

    std::shared_ptr<StaticMesh> PlaneMesh = g_ResourceManager->Load(TEXT("Base/Plane.json"), StaticMesh::GetTypeDescStatic())->CastToShared<StaticMesh>();

    uint32 Index = DebugRenderTargetData[InRenderTarget].Index;
    Vec3 Trans = { -0.5f + (float)Index, 0.f, 1.f };

    FPrimitiveData NewPrimitiveData = {};
    NewPrimitiveData.MeshData = &PlaneMesh->GetMeshData(0);
    NewPrimitiveData.PrimitiveType = EPrimitiveType::CustomPrimitiveType0;
    NewPrimitiveData.Material = DebugRenderTargetData[InRenderTarget].Material;
    NewPrimitiveData.Scale = { 1.f, 1.f, 1.f };
    NewPrimitiveData.Translation = Trans;
    NewPrimitiveData.ProjectionType = EProjectionType::Orthograhpic;

    FMeshBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, PlaneMesh);
    NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[0];
    NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[0];

    GetCurrentScene()->AddTemporalPrimitiveData(NewPrimitiveData);
    //DrawPrimitive(GetWorld().get(), PlaneMesh, Trans, VEC3ZERO, VEC3ONE, EPrimitiveType::Mesh);
#endif
}

void MRenderer::DebugRenderTarget(std::shared_ptr<MRenderTarget> InRederTarget, const Vec3& InTrans)
{
    std::shared_ptr<MMaterial> BaseMat = nullptr;
    std::shared_ptr<MMaterial> NewMat = nullptr;

    g_ResourceManager->Load(TEXT("Base/RenderTarget.json"), BaseMat);
    NewMat = DuplicateObject(BaseMat)->CastToShared<MMaterial>();

    std::shared_ptr<StaticMesh> PlaneMesh = g_ResourceManager->Load(TEXT("Base/Plane.json"), StaticMesh::GetTypeDescStatic())->CastToShared<StaticMesh>();

    FPrimitiveData NewPrimitiveData = {};
    NewPrimitiveData.MeshData = &PlaneMesh->GetMeshData(0);
    NewPrimitiveData.PrimitiveType = EPrimitiveType::CustomPrimitiveType0;
    NewPrimitiveData.Material = NewMat;
    NewPrimitiveData.Scale = { 1.f, 1.f, 1.f };
    NewPrimitiveData.Translation = InTrans;
    NewPrimitiveData.ProjectionType = EProjectionType::Orthograhpic;

    FMeshBufferContainer Buffers;
    getGraphicDevice()->GetBuffers(Buffers, PlaneMesh);
    NewPrimitiveData.VertexBuffer = Buffers.VertexBuffers[0];
    NewPrimitiveData.IndexBuffer = Buffers.IndexBuffers[0];

    GetCurrentScene()->AddTemporalPrimitiveData(NewPrimitiveData);
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
        return GetRenderTarget(InRenderTarget)->AsTexture()->GetShaderResourceView();
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

std::shared_ptr<MRenderPass> MRenderer::GetRenderPass(ERenderPass InRenderPassIndex)
{
    return RenderPasses[(int)InRenderPassIndex];
}

void MRenderer::Render()
{
    if (0 <= DebugRenderTargetIndex && DebugRenderTargetIndex < (int)ERenderTarget::Count)
    {
        DebugRenderTarget((ERenderTarget)DebugRenderTargetIndex);
    }
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

    ResizeRenderTargets(Window->GetID(), 0, 0, Window->GetWidth<uint32>(), Window->GetHeight<uint32>(), Window->IsFullScreen());
    Window->GetOnViewportSizeChangedDelegate().Add(this, &MRenderer::ResizeRenderTargets);
}

void MRenderer::RenderWorld(const std::shared_ptr<MWorld>& InWorld)
{
    assert(InWorld);

    Weights = std::move(MakeGaussianWeights(GaussianRadius, GaussianSigma));
    getGraphicDevice()->UpdateStructuredBuffer(Buffer, Weights.data(), sizeof(float) * Weights.size(), Weights.size(), sizeof(float));

    if (InWorld->getMainCamera() == nullptr)
    {
        return;
    }

    CurrentSceneID = InWorld->GetID();
    auto& Scene = Scenes[CurrentSceneID];

    // RenderScene에서 여기로 옮김
    // 그래야 GetPrimitiveDatas가 정상동작함
    Scene->Begin();

    PerformanceTimer p(TEXT("SceneRenderTime: "));
    RenderScene(Scene);
    SceneRenderTime = p.Record();

    Scene->End();

    InWorld->GetOnRederedDelegate().Broadcast();
}

std::shared_ptr<MWorld> MRenderer::GetWorld()
{
    return Scenes[CurrentSceneID]->GetWorld();
}

void MRenderer::RenderScene(std::unique_ptr<MScene>& InScene)
{
    // HLSL 렌더 옵션
    EConstantBufferLayer Layer = EConstantBufferLayer::Global;
    if (std::shared_ptr<MConstantBuffer>& GlobalBuffer = MShader::GetSharedConstantBuffer(Layer))
    {
        for (auto& Prop : GetTypeDesc()->Properties)
        {
            if (Prop->Type == EType::Bool)
            {
                bool bValue = *static_cast<bool*>(Prop->GetAsVoid(this));
                BOOL Value = bValue ? TRUE : FALSE;
                GlobalBuffer->SetData(StringToWString(Prop->Name), &Value);
            }
            else
            {
                GlobalBuffer->SetData(StringToWString(Prop->Name), Prop->GetAsVoid(this));
            }
        }

        g_pGraphicDevice->SetGlboalConstantBuffer(GlobalBuffer);
    }

    FrustumCulling(InScene);

    Times.clear();

    PerformanceTimer t(TEXT("RenderPassTime: "));
    uint32 RenderPassNum = GetSize(RenderPasses);
    for (uint32 PassIndex = 0; PassIndex < RenderPassNum; ++PassIndex)
    {
        std::shared_ptr<MRenderPass> RenderPass = RenderPasses[PassIndex];
        if (RenderPass == nullptr)
        {
            continue;
        }

        RenderPass->Begin();

        if (bShadowing == false)
        {
            if (PassIndex == EnumToIndex(ERenderPass::PointShadowDepth) || PassIndex == EnumToIndex(ERenderPass::ShadowDepth))
            {
                RenderPass->Clear();
                continue;
            }
        }

        if (bDirectionalLighting == false && PassIndex == EnumToIndex(ERenderPass::DirectionalLight))
        {
            RenderPass->Clear();
            continue;
        }

        if (bPointLighting == false && PassIndex == EnumToIndex(ERenderPass::PointLight))
        {
            RenderPass->Clear();
            continue;
        }

#if RenderPassPerformanceProfiling == 1
        std::wstring Name = TEXT("Pass") + std::to_wstring(PassIndex) + TEXT(" :");
        PerformanceTimer Temp(Name);
        g_pGraphicDevice->QueryStart(PassIndex);
#endif

        RenderPass->RenderPass(InScene->GetRenderablePrimitiveData());

        RenderPass->End();

#if RenderPassPerformanceProfiling == 1
        Times.push_back(Temp.Record());
        g_pGraphicDevice->QueryFinish(PassIndex);
#endif
    }
    RenderPassTime = t.Record();
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

        std::shared_ptr<StaticMeshComponent> StaticMeshComp = PrimitiveDatas[0].PrimitiveComponent.lock()->CastToShared<StaticMeshComponent>();

        for (uint32 i=0; i<GetSize(PrimitiveDatas); ++i)
        {
            const auto& PrimitiveData = PrimitiveDatas[i];
            if (StaticMeshComp && StaticMeshComp->MeshVisibilities[i]== 0)
            {
                continue;
            }

            InScene->AddRenderablePrimitiveData(PrimitiveData);
        }
    }

    auto& TemporalPrimitiveDatas = InScene->GetTemporalPrimitiveDatas();
    for (const auto& PrimitiveData : TemporalPrimitiveDatas)
    {
        std::shared_ptr<MBoundingBox> BoundingBox = nullptr;
        if (BoundingBox) // && BoundingBox->cullSphere() == false
        {
            continue;
        }
        InScene->AddRenderablePrimitiveData(PrimitiveData);
    }
}
