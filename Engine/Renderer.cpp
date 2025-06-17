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
            PointLightPrimitive.push_back(PrimitiveData);
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
    if (PointLightPrimitive.size() > 0)
    {
        const std::shared_ptr<MLightComponent>& InLightComponent = std::static_pointer_cast<MLightComponent>(PointLightPrimitive[0]._pPrimitive.lock());

        XMVECTOR Position = XMLoadFloat3(&InLightComponent->getTranslation());
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
    PointLightPrimitive.clear();
}

void Renderer::RenderScene()
{
	g_pGraphicDevice->Begin();

	TotalPrimitiveNum = CastValue<uint32>(CachedPrimitiveComponents.size());
	FrustumCulling();

    // 무엇을 렌더링 할지          -> RenderPass 안에서 PrimitiveType 이용
    // 어느 시점으로 렌더링 할지   -> 

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

    /*
    // 디렉셔널 그림자 패스
    for (int i = 0; i < 1; ++i)
    {
        std::shared_ptr<DirectionalShadowDepthPass>& Pass = std::static_pointer_cast<DirectionalShadowDepthPass>(RenderPasses[enumToIndex(ERenderPass::ShadowDepth)]);
        Pass->Begin();
        Pass->Render(RenderablePrimitiveData);
        Pass->End();
    }
  
    // 포인트 라이트 그림자 패스
    */

	// 기본 패스
	uint32 combinePassIndex = CastValue<uint32>(ERenderPass::Combine);
	for (uint32 i = 0; i < combinePassIndex; ++i)
	{
		RenderPasses[i]->Begin();
		RenderPasses[i]->Render(RenderablePrimitiveData);
		RenderPasses[i]->End();
	}

	// 혼합 패스
	std::vector<std::weak_ptr<PrimitiveComponent>> temp;
	temp.emplace_back(ViewMeshComponent);

	std::vector<FPrimitiveData> PrimitiveDataList;
	ViewMeshComponent->GetPrimitiveData(PrimitiveDataList);

	if (PrimitiveDataList[0].VertexBuffer == nullptr)
	{
		auto& MeshData = PrimitiveDataList[0].MeshData.lock();
		uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
		uint32 VertexNum = CastValue<uint32>(MeshData->Vertices.size());
		const void* Buffer = MeshData->Vertices.data();
		PrimitiveDataList[0].VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, Buffer);
	}

	RenderPasses[combinePassIndex]->Begin();
	RenderPasses[combinePassIndex]->Render(PrimitiveDataList);
	RenderPasses[combinePassIndex]->End();

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
	const Mat4 &viewMatrix = g_pMainGame->getMainCameraViewMatrix();
	Mat4 projectionMatrix = g_pMainGame->getMainCameraProjectioinMatrix();

	float zMinimum = -projectionMatrix._43 / projectionMatrix._33;
	float r = 1000.f / (1000.f - zMinimum);
	projectionMatrix._33 = r;
	projectionMatrix._43 = -r * zMinimum;
	XMMATRIX proj = XMLoadFloat4x4(&projectionMatrix);
	XMMATRIX viewProj = XMMatrixMultiply(XMLoadFloat4x4(&viewMatrix), proj);

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, viewProj);

	std::vector<XMVECTOR> m_planes(6);
	float x = (float)(matrix._14 + matrix._13);
	float y = (float)(matrix._24 + matrix._23);
	float z = (float)(matrix._34 + matrix._33);
	float w = (float)(matrix._44 + matrix._43);
	m_planes[0] = XMVectorSet(x, y, z, w);
	m_planes[0] = XMPlaneNormalize(m_planes[0]);

	// 절두체의 먼 평면을 계산합니다.
	x = (float)(matrix._14 - matrix._13);
	y = (float)(matrix._24 - matrix._23);
	z = (float)(matrix._34 - matrix._33);
	w = (float)(matrix._44 - matrix._43);
	m_planes[1] = XMVectorSet(x, y, z, w);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// 절두체의 왼쪽 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._11);
	y = (float)(matrix._24 + matrix._21);
	z = (float)(matrix._34 + matrix._31);
	w = (float)(matrix._44 + matrix._41);
	m_planes[2] = XMVectorSet(x, y, z, w);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// 절두체의 오른쪽 평면을 계산합니다.
	x = (float)(matrix._14 - matrix._11);
	y = (float)(matrix._24 - matrix._21);
	z = (float)(matrix._34 - matrix._31);
	w = (float)(matrix._44 - matrix._41);
	m_planes[3] = XMVectorSet(x, y, z, w);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// 절두체의 윗 평면을 계산합니다.
	x = (float)(matrix._14 - matrix._12);
	y = (float)(matrix._24 - matrix._22);
	z = (float)(matrix._34 - matrix._32);
	w = (float)(matrix._44 - matrix._42);
	m_planes[4] = XMVectorSet(x, y, z, w);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// 절두체의 아래 평면을 계산합니다.
	x = (float)(matrix._14 + matrix._12);
	y = (float)(matrix._24 + matrix._22);
	z = (float)(matrix._34 + matrix._32);
	w = (float)(matrix._44 + matrix._42);
	m_planes[5] = XMVectorSet(x, y, z, w);
	m_planes[5] = XMPlaneNormalize(m_planes[5]);

	std::vector<std::weak_ptr<PrimitiveComponent>> CulledPrimitiveComponents;
	CulledPrimitiveComponents.reserve(CachedPrimitiveComponents.size());

	for (auto& WeakPrimitiveComponent : CachedPrimitiveComponents)
	{
		std::shared_ptr<PrimitiveComponent>& SharedPrimitiveComponent = WeakPrimitiveComponent.lock();

		std::shared_ptr<BoundingBox> boundingBox = nullptr;
		if (SharedPrimitiveComponent->getBoundingBox(boundingBox) == false)
		{
			CulledPrimitiveComponents.emplace_back(SharedPrimitiveComponent);
			continue;
		}

		if (boundingBox->cullSphere(m_planes, SharedPrimitiveComponent->getWorldTranslation(), boundingBox->getLength(SharedPrimitiveComponent->getScale())/2.f))
		{
			CulledPrimitiveComponents.emplace_back(SharedPrimitiveComponent);
		}
	}

	RenderablePrimitiveComponents.clear();
	RenderablePrimitiveComponents = std::move(CulledPrimitiveComponents);

	showPrimitiveCount = CastValue<uint32>(CachedPrimitiveComponents.size());
	culledPrimitiveCount = TotalPrimitiveNum - showPrimitiveCount;
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

/*
void Renderer::Test(std::vector<Mat4>& OutLightViewProj, std::vector<Vec4>& lightPosition)
{
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

		XMVECTOR lightDirection = XMVector3Normalize(XMVectorSet(_directionalLightDirection[0].x, _directionalLightDirection[0].y, _directionalLightDirection[0].z, 0.f));
		XMVECTOR CascadeCenterInWorld = XMVector3TransformCoord(CascadeCenter, cameraWorldMatrix);
		XMVECTOR directionalLightPos = CascadeCenterInWorld - (lightDirection * radius);
		XMStoreFloat4(&lightPosition[cascadeIndex], directionalLightPos);

		float Near = std::max(DepthCenter - radius, 0.1f);
		float Far = radius * 2.f;
		XMMATRIX OrthograhpicMatrix = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, Near, Far);

		XMVECTOR UpVector = XMLoadFloat3(&VEC3UP);
		if (fabs(XMVectorGetX(XMVector3Dot(UpVector, lightDirection))) > 0.999f)
		{
			UpVector = XMVectorSet(1.f, 0, 0, 0);
		}
		XMMATRIX LightView = XMMatrixLookAtLH(directionalLightPos, directionalLightPos + lightDirection, UpVector);
		//XMVECTOR projCenter = XMVector3TransformCoord(CascadeCenter, LightView);
		//float worldTexelSize = radius * 2.f / 2048.f;
		//float snapX = roundf(XMVectorGetX(projCenter) / worldTexelSize) * worldTexelSize;
		//float snapY = roundf(XMVectorGetY(projCenter) / worldTexelSize) * worldTexelSize;
		//LightView.r[3].m128_f32[0] += (snapX - XMVectorGetX(projCenter));
		//LightView.r[3].m128_f32[1] += (snapY - XMVectorGetY(projCenter));

		XMMATRIX LightViewProj = XMMatrixMultiply(LightView, OrthograhpicMatrix);
		XMStoreFloat4x4(&OutLightViewProj[cascadeIndex], LightViewProj);

		OutLightViewProj[cascadeIndex]._41 = round(OutLightViewProj[cascadeIndex]._41 * 10.f) / 10.f;
		OutLightViewProj[cascadeIndex]._42 = round(OutLightViewProj[cascadeIndex]._42 * 10.f) / 10.f;

		if (cascadeIndex == 2)
		{
			XMVECTOR Test = XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 10.f, 1.f), LightViewProj);
			Vec3 TestStore;
			XMStoreFloat3(&TestStore, Test);

			std::wostringstream ss;
			ss << TEXT("LightView x:") << TestStore.x << TEXT(", y:") << TestStore.y << TEXT(", z:") << TestStore.z << std::endl;
			OutputDebugString(ss.str().c_str());
		}
	}
}
 */