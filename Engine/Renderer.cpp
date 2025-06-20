#include "Include.h"
#include "Renderer.h"

// DirectXTK
#include "DirectXTK/SpriteFont.h"

// Graphic
#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

// Material
#include "Material.h"

// Render
#include "RenderTarget.h"
#include "RenderPass.h"
#include "CombinePass.h"

// Shader
#include "Shader.h"

// Framework
#include "MainGame.h"
#include "MainGameSetting.h"

#include "PrimitiveComponent.h"
#include "MeshComponent.h"
#include "LightComponent.h"
#include "StaticMeshComponent.h"

#include "Texture.h"

#include "Camera.h"

#include <tuple>

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
	: _drawRenderTarget{ true }
	, _bDirtyConstant{ true }
	, _cascadeDistance(4, 0.f)
    , LightPosition(3, VEC3ZERO)
    , LightViewProj(3, IDENTITYMATRIX)
    , PointLightViewProj(6)
    , PointLightPosition(1)
{
	_cascadeDistance[CastValue<int>(EFrustumCascade::Near)] = 0.1f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Middle)] = 6.f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Middle2)] = 18.f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Far)] = 1000.f;

	_renderTargets.reserve(CastValue<size_t>(ERenderTarget::Count));
	RenderPasses.reserve(CastValue<size_t>(ERenderPass::Count));
	initialize();
}

Renderer::~Renderer() noexcept
{
    Release();
}

void Renderer::Release()
{
	_renderTargets.clear();
	RenderPasses.clear();
	ViewMeshComponent.reset();

	CachedPrimitiveComponents.clear();
	RenderablePrimitiveComponents.clear();
	ForwardPrimitiveDataMap.clear();
	DeferredPrimitiveDataMap.clear();
	VertexBufferMap.clear();
}

void Renderer::initialize() noexcept
{
	ViewMeshComponent = std::make_shared<StaticMeshComponent>(TEXT("Base/Plane.fbx"), false);
	ViewMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 1.f });
	ViewMeshComponent->setScale(Vec3{ g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 1.f });
    ViewMeshComponent->getStaticMesh()->getMaterial(0)->setShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));
	ViewMeshComponent->SceneComponent::Update(0.f);

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
            RenderTargetInfo = FRenderTagetInfo::GetCube();
            RenderTargetInfo.Width = 1024 * 2;
            RenderTargetInfo.Height = 1024 * 2;
            RenderTargetInfo.Type = ERenderTargetType::Depth;
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
	RenderPasses.emplace_back(CreateRenderPass<DirectionalShadowDepthPass>());
	{
		RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->BindRenderTargets(_renderTargets,
			ERenderTarget::DirectionalShadowDepth
		);

		RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->setShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPixel.cso"), TEXT("ShadowDepthGS.cso"));
		RenderPasses[EnumToIndex(ERenderPass::ShadowDepth)]->Color = EngineColors::White;
	}

    RenderPasses.emplace_back(CreateRenderPass<PointShadowDepthPass>());
    {
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->BindRenderTargets(_renderTargets,
            ERenderTarget::PointShadowDepth
        );

        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->setShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPointPS.cso"), TEXT("ShadowDepthPointGS.cso"));
        RenderPasses[EnumToIndex(ERenderPass::PointShadowDepth)]->Color = EngineColors::White;
    }

	RenderPasses.emplace_back(CreateRenderPass<GeometryPass>());
	{
		RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindRenderTargets(_renderTargets,
			ERenderTarget::Diffuse, 
			ERenderTarget::Depth, 
			ERenderTarget::Normal, 
			ERenderTarget::Specular);

        RenderPasses[EnumToIndex(ERenderPass::Geometry)]->BindResourceViews(_renderTargets,
            ERenderTarget::DirectionalShadowDepth,
            ERenderTarget::PointShadowDepth);
	}

	RenderPasses.emplace_back(CreateRenderPass<DirectionalLightPass>());
	{
		RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)]->BindRenderTargets(_renderTargets,
			ERenderTarget::LightDiffuse,
			ERenderTarget::LightSpecular);

		RenderPasses[EnumToIndex(ERenderPass::DirectionalLight)]->BindResourceViews(_renderTargets,
			ERenderTarget::Depth,
			ERenderTarget::Normal,
			ERenderTarget::Specular,
            ERenderTarget::DirectionalShadowDepth);
	}

    RenderPasses.emplace_back(CreateRenderPass<PointLightPass>());
    {
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->BindRenderTargets(_renderTargets,
            ERenderTarget::PointLightDiffuse,
            ERenderTarget::LightSpecular);
        RenderPasses[EnumToIndex(ERenderPass::PointLight)]->BindResourceViews(_renderTargets,
            ERenderTarget::Depth,
            ERenderTarget::Normal,
            ERenderTarget::Specular,
            ERenderTarget::PointShadowDepth);
    }

	RenderPasses.emplace_back(CreateRenderPass<SkyPass>());
	{
		RenderPasses[EnumToIndex(ERenderPass::SkyPass)]->BindRenderTargets(_renderTargets,
			ERenderTarget::Diffuse,
			ERenderTarget::LightDiffuse);

		RenderPasses[EnumToIndex(ERenderPass::SkyPass)]->SetClearTargets(false);
	}

    RenderPasses.emplace_back(CreateRenderPass<CollisionPass>());
    {
        RenderPasses[EnumToIndex(ERenderPass::Collision)]->BindRenderTargets(_renderTargets,
            ERenderTarget::Collision);
    }

	RenderPasses.emplace_back(CreateRenderPass<CombinePass>());
	{
		RenderPasses[EnumToIndex(ERenderPass::Combine)]->BindResourceViews(_renderTargets,
			ERenderTarget::Diffuse,
			ERenderTarget::LightDiffuse,
			ERenderTarget::LightSpecular,
            ERenderTarget::Collision,
            ERenderTarget::PointLightDiffuse);
	}

	// 
	//assert(e && );
	ASSERT_MSG(EnumToIndex(ERenderPass::Count) == static_cast<uint32>(RenderPasses.size()), "ERenderPass::Count와 RenderPasses의 개수가 맞지 않음.");
}

void Renderer::AddPrimitive(std::shared_ptr<PrimitiveComponent>& InPrimitiveComponent)
{
	if (InPrimitiveComponent == nullptr)
	{
		return;
	}

	std::vector<FPrimitiveData> PrimitiveDataList;
	if (InPrimitiveComponent->GetPrimitiveData(PrimitiveDataList) == false)
	{
		return;
	}

	CachedPrimitiveComponents.push_back(InPrimitiveComponent);

	bool bMakeBuffer = false;
	for (int i=0; i<PrimitiveDataList.size(); ++i)
	{
		FPrimitiveData& PrimitiveData = PrimitiveDataList[i];
		if (PrimitiveData.MeshData.expired())
		{
			continue;
		}

		uint32 PrimitiveID = InPrimitiveComponent->GetPrimitiveID();

		// 버텍스 버퍼 생성
		if (bMakeBuffer == false && VertexBufferMap.find(PrimitiveID) == VertexBufferMap.end())
		{
			bMakeBuffer = true;
		}

		if (bMakeBuffer)
		{
			auto& MeshData = PrimitiveData.MeshData.lock();
			uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
			uint32 VertexNum = CastValue<uint32>(MeshData->Vertices.size());
			const void* Buffer = MeshData->Vertices.data();
			VertexBufferMap[PrimitiveID].push_back(std::make_shared<MVertexBuffer>(VertexSize, VertexNum, Buffer));
		}

		PrimitiveData.VertexBuffer = VertexBufferMap[PrimitiveID][i];

		// 인덱스 버퍼 생성
		// --

		// 매터리얼 타입에 따라 어느 렌더링에 들어갈지 결정
		if (bMakeBuffer)
		{
			//if (PrimitiveData._pMaterial->IsUseAlpha())
			//{
			//	ForwardPrimitiveDataMap[PrimitiveKey].push_back(PrimitiveData);
			//}
			//else
			{
				DeferredPrimitiveDataMap[PrimitiveID].push_back(PrimitiveData);
			}
		}

        switch (PrimitiveData._primitiveType)
        {
        case EPrimitiveType::DirectionalLight:
            DirectionalLightPrimitive.push_back(PrimitiveData);
            break;
        case EPrimitiveType::PointLight:
            PointLightPrimitives.push_back(PrimitiveData);
            break;
        default:
            break;
        }
	}
}

void Renderer::addRenderTargetForDebug(const std::wstring name)
{
#ifdef _DEBUG
	//std::shared_ptr<StaticMeshComponent> pMeshComponent = std::make_shared<StaticMeshComponent>();
	//MapUtility::FindInsert(_renderTargetMeshs, name, pMeshComponent);
	//if (true == result)
	//{
	//	float scale = 200.f;
	//	uint32 count = CastValue<uint32>(_renderTargetMeshs.size() - 1);

	//	pMeshComponent->setScale(scale, scale, 0.f);
	//	pMeshComponent->setTranslation(scale / 2.f + (count / 4 * scale), scale / 2.f + (count * scale), 0.f);
	//}
#endif
}

void Renderer::Render()
{
    // 디렉셔널 라이트 데이터 갱신
    if (DirectionalLightPrimitive.size() > 0)
    {
        const std::shared_ptr<MLightComponent>& InLightComponent = std::static_pointer_cast<MLightComponent>(DirectionalLightPrimitive[0]._pPrimitive.lock());

        float tanHalfVertical = tan(XMConvertToRadians(g_pSetting->getFov() / 2.f));
        float tanHalfHorizen = tanHalfVertical * g_pSetting->getAspectRatio();
        XMMATRIX cameraWorldMatrix = XMLoadFloat4x4(&g_pMainGame->getMainCamera()->getInvesrViewMatrix());

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

            // 디버깅용
            //if (cascadeIndex == 2)
            //{
            //    XMVECTOR Test = XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 10.f, 1.f), LightViewProj);
            //    Vec3 TestStore;
            //    XMStoreFloat3(&TestStore, Test);

            //    std::wostringstream ss;
            //    ss << TEXT("LightView x:") << TestStore.x << TEXT(", y:") << TestStore.y << TEXT(", z:") << TestStore.z << std::endl;
            //    OutputDebugString(ss.str().c_str());
            //}
        }
    }

    // 포인트 라이트 데이터 갱신
    for(auto& PointLightPrimitive : PointLightPrimitives)
    {
        const std::shared_ptr<MLightComponent>& LightComponent = std::static_pointer_cast<MLightComponent>(PointLightPrimitive._pPrimitive.lock());

        XMVECTOR Position = XMLoadFloat3(&LightComponent->getTranslation());
        XMStoreFloat3(&PointLightPosition[0], Position);

        XMVECTOR Up = XMLoadFloat3(&VEC3UP);

        XMMATRIX Proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, 0.1f, 1000.f);

        XMStoreFloat4x4(&PointLightViewProj[0], XMMatrixLookAtLH(Position, Position + XMVectorSet(1.f, 0.f, 0.f, 0.f), Up) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[1], XMMatrixLookAtLH(Position, Position + XMVectorSet(-1.f, 0.f, 0.f, 0.f), Up) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[2], XMMatrixLookAtLH(Position, Position + XMVectorSet(0.f, 1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 1.f)) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[3], XMMatrixLookAtLH(Position, Position + XMVectorSet(0.f, -1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 1.f)) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[4], XMMatrixLookAtLH(Position, Position + XMVectorSet(0.f, 0.f, 1.f, 0.f), Up) * Proj);
        XMStoreFloat4x4(&PointLightViewProj[5], XMMatrixLookAtLH(Position, Position + XMVectorSet(0.f, 0.f, -1.f, 0.f), Up) * Proj);
    }

    for (uint32 index = 0; index < CastValue<uint32>(ShaderType::Count); ++index)
    {
        auto& Shaders = ShaderManager->GetShaders(CastValue<ShaderType>(index));

        for (auto& Pair : Shaders)
        {
            UpdateGlobalConstantBuffer(Pair.second);
            UpdateTickConstantBuffer(Pair.second);
        }
    }

	RenderScene();
	RenderText();

    DirectionalLightPrimitive.clear();
    PointLightPrimitives.clear();
}

void Renderer::RenderScene()
{
	g_pGraphicDevice->Begin();

	TotalPrimitiveNum = CastValue<uint32>(CachedPrimitiveComponents.size());
	FrustumCulling();

    // 렌더링 할 Prmitive 선별
	std::vector<FPrimitiveData> RenderablePrimitiveData;
	for (auto& Component : RenderablePrimitiveComponents)
	{
		uint32 ID = Component.lock()->GetPrimitiveID();
		if (DeferredPrimitiveDataMap.find(ID) == DeferredPrimitiveDataMap.end())
		{
            continue;
		}

		RenderablePrimitiveData.insert(RenderablePrimitiveData.end(), DeferredPrimitiveDataMap[ID].begin(), DeferredPrimitiveDataMap[ID].end());
	}

	// 기본 패스
	uint32 CombinePass = EnumToIndex(ERenderPass::Combine);
	for (uint32 PassIndex = 0; PassIndex < CombinePass; ++PassIndex)
	{
		RenderPasses[PassIndex]->Begin();
		RenderPasses[PassIndex]->Render(RenderablePrimitiveData);
		RenderPasses[PassIndex]->End();
	}

	// 혼합 패스
	std::vector<FPrimitiveData> ViewPrimitiveData;
	ViewMeshComponent->GetPrimitiveData(ViewPrimitiveData);

	if (ViewPrimitiveData[0].VertexBuffer == nullptr)
	{
		auto& MeshData = ViewPrimitiveData[0].MeshData.lock();
		uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
		uint32 VertexNum = CastValue<uint32>(MeshData->Vertices.size());
		const void* Buffer = MeshData->Vertices.data();
		ViewPrimitiveData[0].VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, Buffer);
	}

	RenderPasses[CombinePass]->Begin();
	RenderPasses[CombinePass]->Render(ViewPrimitiveData);
	RenderPasses[CombinePass]->End();

	// 포스트프로세스 패스


	// 렌더 타겟
#ifdef _DEBUG
	//if (true == _drawRenderTarget)
	//{
	//	for each (auto pair in _renderTargetMeshMap)
	//	{
	//		pair.second->render();
	//	}
	//}
#endif

	g_pMainGame->render();
	CachedPrimitiveComponents.clear();

	g_pGraphicDevice->End();
}

void Renderer::RenderText()
{

}

void Renderer::FrustumCulling()
{
    const std::shared_ptr<MCamera>& Camera = g_pMainGame->getMainCamera();

	XMMATRIX ViewProj = XMMatrixMultiply(XMLoadFloat4x4(&Camera->getViewMatrix()), XMLoadFloat4x4(&g_pMainGame->getMainCameraProjectioinMatrix()));

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

	std::vector<std::weak_ptr<PrimitiveComponent>> ShownPrimitiveComponents;
	ShownPrimitiveComponents.reserve(CachedPrimitiveComponents.size());

	for (auto& WeakPrimitiveComponent : CachedPrimitiveComponents)
	{
		std::shared_ptr<PrimitiveComponent>& SharedPrimitiveComponent = WeakPrimitiveComponent.lock();

		std::shared_ptr<BoundingBox> boundingBox = nullptr;
		if (SharedPrimitiveComponent->GetBoundingBox(boundingBox) == false)
		{
			ShownPrimitiveComponents.emplace_back(SharedPrimitiveComponent);
			continue;
		}

		if (boundingBox->cullSphere(Planes, SharedPrimitiveComponent->getWorldTranslation(), boundingBox->getLength(SharedPrimitiveComponent->getScale())/2.f))
		{
			ShownPrimitiveComponents.emplace_back(SharedPrimitiveComponent);
		}
	}

    ShownPrimitiveNum = CastValue<uint32>(ShownPrimitiveComponents.size());
    CulledPrimitiveNum = TotalPrimitiveNum - ShownPrimitiveNum;

	RenderablePrimitiveComponents.clear();
	RenderablePrimitiveComponents = std::move(ShownPrimitiveComponents);
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

    Shader->SetValue(TEXT("PointLightPos"), PointLightPosition);
    Shader->SetValue(TEXT("PointLightViewProj"), PointLightViewProj);
}

void Renderer::toggleRenderTarget()
{
	_drawRenderTarget = (true == _drawRenderTarget) ? false : true;
}

const bool Renderer::IsGlobalBufferDirty() const
{
	return _bDirtyConstant;
}