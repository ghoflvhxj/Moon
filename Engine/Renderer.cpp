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

#undef max
#undef min

using namespace DirectX;

enum class EFrustumCascade
{
    Near,
    Middle,
    Middle2,
    Far,
    Count
};

Renderer::Renderer() noexcept
	: _bDirtyConstant{ true }
	, _cascadeDistance(4, 0.f)
    , LightPosition(3, VEC3ZERO)
    , LightViewProj(3, IDENTITYMATRIX)
{
	_cascadeDistance[CastValue<int>(EFrustumCascade::Near)] = 0.1f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Middle)] = 6.f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Middle2)] = 18.f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Far)] = 1000.f;

	_renderTargets.reserve(CastValue<size_t>(ERenderTarget::Count));
	RenderPasses.resize(CastValue<size_t>(ERenderPass::Count), nullptr);

    GetLevelChangedDelegate().Add([&]() {
        RenderablePrimitiveData.clear();
        PrimitiveDatasPerType.clear();

        PrimitiveComponents.clear();

        IndexBuffers.clear();
        VertexBuffers.clear();

        IdToPrimitiveDatas.clear();

        ViewPrimitiveData.clear();
        if (ViewMeshComponent)
        {
            ViewMeshComponent->GetPrimitiveData(ViewPrimitiveData);
            MakeBuffer(ViewMeshComponent);
        }

        if (GizmoMeshComp)
        {
            MakeBuffer(GizmoMeshComp);
        }

        for (auto& DebugMesh : DebugRenderTargetMehses)
        {
            MakeBuffer(DebugMesh.second);
        }
    });
}

Renderer::~Renderer() noexcept
{
    Release();
}

bool Renderer::Initialize()
{
    Super::Initialize();

    ViewMeshComponent = std::make_shared<StaticMeshComponent>();
    ViewMeshComponent->SetPhysics(false);
    ViewMeshComponent->SetMesh(TEXT("Base/Plane.fbx"));
    ViewMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 1.f });
    ViewMeshComponent->setScale(Vec3{ g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 1.f });
    ViewMeshComponent->GetMesh()->getMaterial(0)->setShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));

    std::shared_ptr<MMaterial> ViewMat = nullptr;
    g_ResourceManager->Load(TEXT("Base/Deferred.json"), ViewMat);
    ViewMeshComponent->SetMaterial(0, ViewMat);

    ViewMeshComponent->SceneComponent::Update(0.f);
    ViewMeshComponent->GetPrimitiveData(ViewPrimitiveData);
    MakeBuffer(ViewMeshComponent);

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
            RenderTargetInfo.TextrueNum = CastValue<int>(_cascadeDistance.size());
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
        case ERenderTarget::Depth:
        {
            RenderTargetInfo = FRenderTagetInfo::GetDefault();
            RenderTargetInfo.Type = ERenderTargetType::Depth;
        }
		default:
		{
			RenderTargetInfo = FRenderTagetInfo::GetDefault();
		}
		break;
		}

		_renderTargets.emplace_back(std::make_shared<RenderTarget>(RenderTargetInfo));
	}

	// 렌더 패스 추가
	RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)] = CreateRenderPass<DirectionalShadowDepthPass>();
	{
		RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->BindRenderTargets(_renderTargets,
			ERenderTarget::DirectionalShadowDepth
		);

		RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->setShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPixel.cso"), TEXT("ShadowDepthGS.cso"));
		RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->Color = EngineColors::White;
	}

    RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)] = CreateRenderPass<PointShadowDepthPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->BindRenderTargets(_renderTargets,
            ERenderTarget::PointShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->setShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPointPS.cso"), TEXT("ShadowDepthPointGS.cso"));
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->Color = EngineColors::White;
    }

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

    RenderPasses[EnumToIndex(ERenderPass::Stencil)] = CreateRenderPass<MRenderPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindResourceViews(_renderTargets,
            ERenderTarget::Stencil
        );
    }

    RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)] =CreateRenderPass<DirectionalLightPass>();
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

    RenderPasses[EnumToIndex(ERenderPass::Collision)] = CreateRenderPass<CollisionPass>();
    {
        RenderPasses[EnumToIndex(ERenderPass::Collision)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Collision
        );
    }

    RenderPasses[EnumToIndex(ERenderPass::Combine)] = CreateRenderPass<CombinePass>();
	{
		RenderPasses[EnumToIndex(ERenderPass::Combine)]->BindResourceViews(_renderTargets,
			ERenderTarget::Diffuse,
			ERenderTarget::LightDiffuse,
			ERenderTarget::LightSpecular,
            ERenderTarget::Collision,
            ERenderTarget::PointLightDiffuse
        );
	}

    RenderPasses[EnumToIndex(ERenderPass::Test)] = CreateRenderPass<MRenderPass>();
	{
        RenderPasses[EnumToIndex(ERenderPass::Test)]->SetDepthEnable(false);
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
    MakeBuffer(GizmoMeshComp);

    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::Diffuse)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::Depth)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::Normal)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::Specular)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::LightDiffuse)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::LightSpecular)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::DirectionalShadowDepth)->AsTexture());
    //addRenderTargetForDebug(GetRenderTarget(ERenderTarget::PointShadowDepth)->AsTexture());
    //addRenderTargetForDebug(g_pGraphicDevice->StencilTexture);

    return EnumToIndex(ERenderPass::Count) == GetSize(RenderPasses);
}

void Renderer::Release()
{
    Super::Release();

    _renderTargets.clear();
    RenderPasses.clear();

    // 객체가 삭제되는 것이 아니기에 여기서 직접 해제해줘야 메모리 로그가 안남음
    ViewMeshComponent.reset();
    GizmoMeshComp.reset();

    RenderablePrimitiveData.clear();
    PrimitiveDatasPerType.clear();

    PrimitiveComponents.clear();

    IndexBuffers.clear();
    VertexBuffers.clear();

    IdToPrimitiveDatas.clear();
}

void Renderer::AddPrimitive(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent)
{
	if (InPrimitiveComponent == nullptr)
	{
		return;
	}

    uint32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();

    PrimitiveComponents[PrimitiveID] = InPrimitiveComponent;
    //Primitives[PrimitiveData.PrimitiveType].push_back(PrimitiveData);

    MakeBuffer(InPrimitiveComponent);
    if (std::shared_ptr<MMeshComponent> MeshComp = InPrimitiveComponent->CastTo<MMeshComponent>())
    {
        MeshComp->GetMeshChangedDelegate().Add(this, &Renderer::MakeBuffer);
    }

    MakePrimitiveData(InPrimitiveComponent);
    InPrimitiveComponent->GetPrimitiveChangedDelegate().Add(this, &Renderer::MakePrimitiveData);
}

void Renderer::MakePrimitiveData(std::shared_ptr<MPrimitiveComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    int32 PrimitiveID = InComponent->GetPrimitiveID();
    IdToPrimitiveDatas.erase(PrimitiveID);

    std::vector<FPrimitiveData> PrimitiveDatas;
    if (InComponent->GetPrimitiveData(PrimitiveDatas) == false)
    {
        return;
    }

    for (uint32 i = 0; i < GetSize(PrimitiveDatas); ++i)
    {
        FPrimitiveData& PrimitiveData = PrimitiveDatas[i];
        PrimitiveData.VertexBuffer = VertexBuffers[PrimitiveID][i];
        PrimitiveData.IndexBuffer = IndexBuffers[PrimitiveID][i];

        if (PrimitiveData.MeshData == nullptr)
        {
            continue;
        }

        IdToPrimitiveDatas[PrimitiveID].push_back(PrimitiveData);
        PrimitiveDatasPerType[PrimitiveData.PrimitiveType].push_back(PrimitiveData);
    }
}

void Renderer::MakeBuffer(FPrimitiveData& PrimitiveData)
{
	int32 PrimitiveID = PrimitiveData.PrimitiveComponent.lock()->GetPrimitiveID();

	auto& MeshData = *PrimitiveData.MeshData;
	uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
	uint32 VertexNum = GetSize(MeshData.Vertices);
	VertexBuffers[PrimitiveID].push_back(std::make_shared<MVertexBuffer>(VertexSize, VertexNum, MeshData.Vertices.data()));

	uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
	uint32 IndexNum = GetSize(MeshData.Indices);
	IndexBuffers[PrimitiveID].push_back(IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, MeshData.Indices.data()) : nullptr);
}

void Renderer::MakeBuffer(std::shared_ptr<MPrimitiveComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }

    int32 PrimitiveID = InComponent->GetPrimitiveID();

    VertexBuffers.erase(PrimitiveID);
    IndexBuffers.erase(PrimitiveID);

    std::vector<FPrimitiveData> PrimitiveDatas;
    InComponent->GetPrimitiveData(PrimitiveDatas);

    for (auto& PrimtiveData : PrimitiveDatas)
    {
        MakeBuffer(PrimtiveData);
    }
}

void Renderer::addRenderTargetForDebug(ERenderTarget InRenderTarget)
{
#ifdef _DEBUG
    std::shared_ptr<MMaterial> BaseMat = nullptr;
    g_ResourceManager->Load(TEXT("Base/RenderTarget.json"), BaseMat);

	float scale = 200.f;
	float x = (-1.f * g_pSetting->getResolutionWidth<float>() / 2.f) + (scale / 2.f);
	float y = g_pSetting->getResolutionHeight<float>();

    auto& NewMesh = std::make_shared<StaticMeshComponent>();
	NewMesh->SetPhysics(false);
    NewMesh->SetMesh(TEXT("Base/Plane.fbx"));
    if (MapUtility::FindInsert(DebugRenderTargetMehses, InRenderTarget, NewMesh))
    {
        
        uint32 count = CastValue<uint32>(DebugRenderTargetMehses.size() - 1);
        NewMesh->setScale(scale, scale, 0.f);
        NewMesh->setTranslation(x + scale * count, scale, 1.f);

        std::shared_ptr<MMaterial> NewMat = std::make_shared<MMaterial>();
        *NewMat.get() = *BaseMat.get();
        NewMat->setTexture(ETextureType::Diffuse, GetRenderTarget(InRenderTarget)->AsTexture());

		NewMesh->SetMaterial(0, NewMat);
        NewMesh->setRenderMode(MPrimitiveComponent::ERenderMode::Orthogonal);
		NewMesh->SceneComponent::Update(0.f);

		std::vector<FPrimitiveData> PrimitiveDatas;
		NewMesh->GetPrimitiveData(PrimitiveDatas);
		for (auto& PrimitiveData : PrimitiveDatas)
		{
			MakeBuffer(PrimitiveData);
		}
    }
#endif
}

void Renderer::Render()
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
            float Depth = _cascadeDistance[cascadeIndex];
            float NextDepth = _cascadeDistance[cascadeIndex + 1];
            float XNear = _cascadeDistance[cascadeIndex] * tanHalfHorizen;
            float XFar = _cascadeDistance[cascadeIndex + 1] * tanHalfHorizen;
            float YNear = _cascadeDistance[cascadeIndex] * tanHalfVertical;
            float YFar = _cascadeDistance[cascadeIndex + 1] * tanHalfVertical;
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
            XMStoreFloat3(&LightPosition[cascadeIndex], LightPositionInFrustume);

            float Near = std::max(DepthCenter - radius, 0.1f);
            float Far = radius * 2.f;
            XMMATRIX OrthograhpicMatrix = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, Near, Far);

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
            XMMATRIX XMMatLightViewProj = XMMatrixMultiply(LightView, OrthograhpicMatrix);
            XMStoreFloat4x4(&LightViewProj[cascadeIndex], XMMatLightViewProj);
            LightViewProj[cascadeIndex]._42 = round(LightViewProj[cascadeIndex]._42 * 10.f) / 10.f;
            LightViewProj[cascadeIndex]._43 = round(LightViewProj[cascadeIndex]._43 * 10.f) / 10.f;
        }
    }

    for (uint32 index = 0; index < CastValue<uint32>(ShaderType::Count); ++index)
    {
        auto& Shaders = g_pGraphicDevice->GetShaderManager()->GetShaders(CastValue<ShaderType>(index));

        for (auto& Pair : Shaders)
        {
            UpdateGlobalConstantBuffer(Pair.second);
            UpdateTickConstantBuffer(Pair.second);
        }
    }

	RenderScene();

    std::vector<FPrimitiveData> PostRenderPrimitiveDatas;

    if (bGizmo)
    {
        float DistToScale = XMVectorGetX(XMVector3Length(XMLoadFloat3(&g_World->getMainCamera()->GetWorldTranslation()) - XMLoadFloat3(&GizmoPos))) / 10.f;

        std::vector<FPrimitiveData> GizmoPrimitives;
        GizmoMeshComp->setTranslation(GizmoPos);
        GizmoMeshComp->setScale(0.001f * DistToScale, 0.001f * DistToScale, 0.001f * DistToScale);
        GizmoMeshComp->GetPrimitiveData(GizmoPrimitives);
        GizmoMeshComp->SceneComponent::Update(0.f);

        for (uint32 i = 0; i < GetSize(GizmoPrimitives); ++i)
        {
            uint32 PrimitiveID = GizmoMeshComp->GetPrimitiveID();
            auto& GizmoPrimitive = GizmoPrimitives[i];
            GizmoPrimitive.VertexBuffer = VertexBuffers[PrimitiveID][i];
            GizmoPrimitive.IndexBuffer = IndexBuffers[PrimitiveID][i];
        }

        PostRenderPrimitiveDatas.insert(PostRenderPrimitiveDatas.end(), GizmoPrimitives.begin(), GizmoPrimitives.end());
    }

#ifdef _DEBUG
    // 렌더 타겟
    if (true == bDebugRenderTargets)
    {
        for (auto pair : DebugRenderTargetMehses)
        {
            auto& RenderTargetMesh = pair.second;

            RenderTargetMesh->GetPrimitiveData(PostRenderPrimitiveDatas);
            PostRenderPrimitiveDatas.back().VertexBuffer = VertexBuffers[RenderTargetMesh->GetPrimitiveID()][0];
            PostRenderPrimitiveDatas.back().IndexBuffer = IndexBuffers[RenderTargetMesh->GetPrimitiveID()][0];
        }
    }
#endif

    RenderPasses[(int)ERenderPass::Test]->RenderPass(PostRenderPrimitiveDatas);

	RenderText();

    g_World->render();
}

void Renderer::RenderScene()
{
	TotalPrimitiveNum = GetSize(PrimitiveComponents);
	FrustumCulling();

	// 기본 패스
	uint32 CombinePass = EnumToIndex(ERenderPass::Combine);
	for (uint32 PassIndex = 0; PassIndex < CombinePass; ++PassIndex)
	{
        if (std::shared_ptr<MRenderPass>& CurrentRenderPass = RenderPasses[PassIndex])
        {
            CurrentRenderPass->RenderPass(RenderablePrimitiveData);
        }
	}

	// 혼합 패스
    ViewPrimitiveData[0].VertexBuffer = VertexBuffers[ViewMeshComponent->GetPrimitiveID()][0];
    ViewPrimitiveData[0].IndexBuffer = IndexBuffers[ViewMeshComponent->GetPrimitiveID()][0];
    RenderPasses[CombinePass]->RenderPass(ViewPrimitiveData);
}

void Renderer::RenderText()
{

}

void Renderer::FrustumCulling()
{
    RenderablePrimitiveData.clear();

    const std::shared_ptr<MCamera>& Camera = g_World->getMainCamera();

	XMMATRIX ViewProj = XMMatrixMultiply(XMLoadFloat4x4(&Camera->getViewMatrix()), XMLoadFloat4x4(&g_World->getMainCameraProjectioinMatrix()));

	XMFLOAT4X4 ViewProjectMatrix;
	XMStoreFloat4x4(&ViewProjectMatrix, ViewProj);

    // 평면의 방정식 ax + by + cz + d = 0을 구해야 함

    // 절두체 Near 평면
	std::vector<XMVECTOR> Planes(6);
    float a = ViewProjectMatrix._13;
    float b = ViewProjectMatrix._23;
    float c = ViewProjectMatrix._33;
    float d = ViewProjectMatrix._43;
	Planes[0] = XMVectorSet(a, b, c, d);
	Planes[0] = XMPlaneNormalize(Planes[0]);

	// 절두체 Far 평면
	a = (float)(ViewProjectMatrix._14 - ViewProjectMatrix._13);
	b = (float)(ViewProjectMatrix._24 - ViewProjectMatrix._23);
	c = (float)(ViewProjectMatrix._34 - ViewProjectMatrix._33);
	d = (float)(ViewProjectMatrix._44 - ViewProjectMatrix._43);
	Planes[1] = XMVectorSet(a, b, c, d);
	Planes[1] = XMPlaneNormalize(Planes[1]);

	// 절두체의 왼쪽 평면
	a = (float)(ViewProjectMatrix._14 + ViewProjectMatrix._11);
	b = (float)(ViewProjectMatrix._24 + ViewProjectMatrix._21);
	c = (float)(ViewProjectMatrix._34 + ViewProjectMatrix._31);
	d = (float)(ViewProjectMatrix._44 + ViewProjectMatrix._41);
	Planes[2] = XMVectorSet(a, b, c, d);
	Planes[2] = XMPlaneNormalize(Planes[2]);

	// 절두체의 오른쪽 평면
	a = (float)(ViewProjectMatrix._14 - ViewProjectMatrix._11);
	b = (float)(ViewProjectMatrix._24 - ViewProjectMatrix._21);
	c = (float)(ViewProjectMatrix._34 - ViewProjectMatrix._31);
	d = (float)(ViewProjectMatrix._44 - ViewProjectMatrix._41);
	Planes[3] = XMVectorSet(a, b, c, d);
	Planes[3] = XMPlaneNormalize(Planes[3]);

	// 절두체의 윗 평면
	a = (float)(ViewProjectMatrix._14 - ViewProjectMatrix._12);
	b = (float)(ViewProjectMatrix._24 - ViewProjectMatrix._22);
	c = (float)(ViewProjectMatrix._34 - ViewProjectMatrix._32);
	d = (float)(ViewProjectMatrix._44 - ViewProjectMatrix._42);
	Planes[4] = XMVectorSet(a, b, c, d);
	Planes[4] = XMPlaneNormalize(Planes[4]);

	// 절두체의 아래 평면
	a = (float)(ViewProjectMatrix._14 + ViewProjectMatrix._12);
	b = (float)(ViewProjectMatrix._24 + ViewProjectMatrix._22);
	c = (float)(ViewProjectMatrix._34 + ViewProjectMatrix._32);
	d = (float)(ViewProjectMatrix._44 + ViewProjectMatrix._42);
	Planes[5] = XMVectorSet(a, b, c, d);
	Planes[5] = XMPlaneNormalize(Planes[5]);

    //RenderablePrimitiveComponents.clear();
    TotalPrimitiveNum = 0;
    CulledPrimitiveNum = 0;
    ShownPrimitiveNum = 0;

    for (auto& [Id, PrimitiveDatas] : IdToPrimitiveDatas)
    {
        TotalPrimitiveNum += GetSize(PrimitiveDatas);

        const std::shared_ptr<MPrimitiveComponent>& PrimitiveComponent = PrimitiveDatas[0].PrimitiveComponent.lock();

        std::shared_ptr<MBoundingBox> BoundingBox = nullptr;
        // 바운딩 박스가 없으면 일단 무조건 렌더링
        if (PrimitiveComponent->GetBoundingBox(BoundingBox))
        {
            // 컬링
            if (BoundingBox->cullSphere(Planes, PrimitiveComponent->getWorldTranslation(), BoundingBox->GetLength(PrimitiveComponent->getScale()) / 2.f) == false)
            {
                CulledPrimitiveNum += GetSize(PrimitiveDatas);
                continue;
            }
        }

        RenderablePrimitiveData.insert(RenderablePrimitiveData.end(), PrimitiveDatas.begin(), PrimitiveDatas.end());
        ShownPrimitiveNum += GetSize(PrimitiveDatas);
    }
}

void Renderer::UpdateGlobalConstantBuffer(std::shared_ptr<MShader>& Shader)
{
    Vec4 resolution = { g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 0.f, 0.f };
    Shader->SetValue(TEXT("resolution"), resolution);

    BOOL bLight = TRUE;
    Shader->SetValue(TEXT("bLight"), bLight);
}

void Renderer::UpdateTickConstantBuffer(std::shared_ptr<MShader>& Shader)
{
    Shader->SetValue(TEXT("cascadeDistance"), _cascadeDistance);
    Shader->SetValue(TEXT("lightPos"), LightPosition);
    Shader->SetValue(TEXT("lightViewProjMatrix"), LightViewProj);
}

const bool Renderer::IsGlobalBufferDirty() const
{
	return _bDirtyConstant;
}