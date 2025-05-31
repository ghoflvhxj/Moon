#include "Include.h"
#include "Renderer.h"

// DirectXTK
#include <SpriteFont.h>

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

#include "TextureComponent.h"

#include "Camera.h"

#include <tuple>

using namespace DirectX;

enum class EFrustumCascade
{
	Near,
	Middle,
	Middle2,
	Far,
	Count
};

Renderer::Renderer(void) noexcept
	: _drawRenderTarget{ true }
	, _bDirtyConstant{ true }
	, _cascadeDistance(4, 0.f)
{
	_cascadeDistance[CastValue<int>(EFrustumCascade::Near)] = 0.1f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Middle)] = 6.f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Middle2)] = 18.f;
	_cascadeDistance[CastValue<int>(EFrustumCascade::Far)] = 1000.f;

	_renderTargets.reserve(CastValue<size_t>(ERenderTarget::Count));
	_renderPasses.reserve(CastValue<size_t>(ERenderPass::Count));
	initialize();
}

Renderer::~Renderer() noexcept
{
	Release();
}

void Renderer::Release()
{
	_renderTargets.clear();
	_renderPasses.clear();
	_pMeshComponent.reset();
}

void Renderer::initialize(void) noexcept
{
	std::shared_ptr<TextureComponent> _pTextureComponent = std::make_shared<TextureComponent>(TEXT("./Resources/Texture/Player.jpeg"));

	_pMeshComponent = std::make_shared<StaticMeshComponent>("Base/Plane.fbx", false);
	_pMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 1.f });
	_pMeshComponent->setScale(Vec3{ CastValue<float>(g_pSetting->getResolutionWidth()), CastValue<float>(g_pSetting->getResolutionHeight()), 1.f });
	_pMeshComponent->SceneComponent::Update(0.f);

	RenderTagetInfo shadowMapInfo = {};
	shadowMapInfo.width = 1920;
	shadowMapInfo.height = 1080;
	shadowMapInfo.textureArrayCount = CastValue<int>(_cascadeDistance.size());

	for (int i = 0; i < CastValue<int>(ERenderTarget::Count); ++i)
	{
		switch (CastValue<ERenderTarget>(i))
		{
		case ERenderTarget::ShadowDepth:
			_renderTargets.emplace_back(std::make_shared<RenderTarget>(shadowMapInfo));
			break;
		default:
			_renderTargets.emplace_back(std::make_shared<RenderTarget>());
			break;
		}
	}

	_renderPasses.emplace_back(CreateRenderPass<ShadowDepthPass>());
	{
		_renderPasses.back()->SetUseOwningDepthStencilBuffer(true);
		_renderPasses[enumToIndex(ERenderPass::ShadowDepth)]->initializeRenderTargets(_renderTargets,
			ERenderTarget::ShadowDepth
		);

		_renderPasses[enumToIndex(ERenderPass::ShadowDepth)]->setShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPixel.cso"), TEXT("ShadowDepthGS.cso"));
	}

	_renderPasses.emplace_back(CreateRenderPass<GeometryPass>());
	{
		_renderPasses[enumToIndex(ERenderPass::Geometry)]->initializeRenderTargets(_renderTargets,
			ERenderTarget::Diffuse, 
			ERenderTarget::Depth, 
			ERenderTarget::Normal, 
			ERenderTarget::Specular);

		_renderPasses[enumToIndex(ERenderPass::Geometry)]->initializeResourceViews(_renderTargets,
			ERenderTarget::ShadowDepth);
	}

	_renderPasses.emplace_back(CreateRenderPass<LightPass>());
	{
		_renderPasses[enumToIndex(ERenderPass::Light)]->initializeRenderTargets(_renderTargets,
			ERenderTarget::LightDiffuse,
			ERenderTarget::LightSpecular);

		_renderPasses[enumToIndex(ERenderPass::Light)]->initializeResourceViews(_renderTargets,
			ERenderTarget::Depth,
			ERenderTarget::Normal,
			ERenderTarget::Specular);
	}

	_renderPasses.emplace_back(CreateRenderPass<SkyPass>());
	{
		_renderPasses[enumToIndex(ERenderPass::SkyPass)]->initializeRenderTargets(_renderTargets,
			ERenderTarget::Diffuse,
			ERenderTarget::LightDiffuse);

		_renderPasses[enumToIndex(ERenderPass::SkyPass)]->SetClearTargets(false);
	}

	_renderPasses.emplace_back(CreateRenderPass<CombinePass>());
	{
		_renderPasses[enumToIndex(ERenderPass::Combine)]->initializeResourceViews(_renderTargets,
			ERenderTarget::Diffuse,
			ERenderTarget::LightDiffuse,
			ERenderTarget::LightSpecular);

		_renderPasses[enumToIndex(ERenderPass::Combine)]->setShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));
	}

	// 
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

		uint32 PrimitiveKey = InPrimitiveComponent->GetPrimitiveID();

		// ���ؽ� ���� ����
		if (bMakeBuffer == false && VertexBufferMap.find(PrimitiveKey) == VertexBufferMap.end())
		{
			bMakeBuffer = true;
		}

		if (bMakeBuffer)
		{
			auto& MeshData = PrimitiveData.MeshData.lock();
			uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
			uint32 VertexNum = CastValue<uint32>(MeshData->Vertices.size());
			const void* Buffer = MeshData->Vertices.data();
			VertexBufferMap[PrimitiveKey].push_back(std::make_shared<VertexBuffer>(VertexSize, VertexNum, Buffer));
		}

		PrimitiveData._pVertexBuffer = VertexBufferMap[PrimitiveKey][i];

		// �ε��� ���� ����
		// --

		// ���͸��� Ÿ�Կ� ���� ��� �������� ���� ����
		if (bMakeBuffer)
		{
			if (PrimitiveData._pMaterial->IsUseAlpha())
			{
				ForwardPrimitiveDataMap[PrimitiveKey].push_back(PrimitiveData);
			}
			else
			{
				DeferredPrimitiveDataMap[PrimitiveKey].push_back(PrimitiveData);
			}
		}
	}
}

void Renderer::addDirectionalLightInfoForShadow(const Vec3 &direction)
{
	_directionalLightDirection.emplace_back(direction);
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

void Renderer::render()
{
	renderScene();
	renderText();
}

void Renderer::renderScene()
{
	g_pGraphicDevice->Begin();

	TotalPrimitiveNum = CastValue<uint32>(CachedPrimitiveComponents.size());
	FrustumCulling();

	updateConstantBuffer();
	
	std::vector<FPrimitiveData> RenderablePrimitiveData;
	for (auto& Component : RenderablePrimitiveComponents)
	{
		uint32 ID = Component.lock()->GetPrimitiveID();
		if (DeferredPrimitiveDataMap.find(ID) != DeferredPrimitiveDataMap.end())
		{
			RenderablePrimitiveData.insert(RenderablePrimitiveData.end(), DeferredPrimitiveDataMap[ID].begin(), DeferredPrimitiveDataMap[ID].end());
		}
	}

	// �⺻ �н�
	uint32 combinePassIndex = CastValue<uint32>(ERenderPass::Combine);
	for (uint32 i = 0; i < combinePassIndex; ++i)
	{
		_renderPasses[i]->Begin();
		_renderPasses[i]->DoPass(RenderablePrimitiveData);
		_renderPasses[i]->End();
	}

	// ȥ�� �н�
	std::vector<std::weak_ptr<PrimitiveComponent>> temp;
	temp.emplace_back(_pMeshComponent);

	std::vector<FPrimitiveData> PrimitiveDataList;
	_pMeshComponent->GetPrimitiveData(PrimitiveDataList);

	if (PrimitiveDataList[0]._pVertexBuffer == nullptr)
	{
		auto& MeshData = PrimitiveDataList[0].MeshData.lock();
		uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
		uint32 VertexNum = CastValue<uint32>(MeshData->Vertices.size());
		const void* Buffer = MeshData->Vertices.data();
		PrimitiveDataList[0]._pVertexBuffer = std::make_shared<VertexBuffer>(VertexSize, VertexNum, Buffer);
	}

	_renderPasses[combinePassIndex]->Begin();
	_renderPasses[combinePassIndex]->DoPass(PrimitiveDataList);
	_renderPasses[combinePassIndex]->End();

	// ����Ʈ���μ��� �н�


	// ���� Ÿ��
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

void Renderer::renderText()
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

	// ����ü�� �� ����� ����մϴ�.
	x = (float)(matrix._14 - matrix._13);
	y = (float)(matrix._24 - matrix._23);
	z = (float)(matrix._34 - matrix._33);
	w = (float)(matrix._44 - matrix._43);
	m_planes[1] = XMVectorSet(x, y, z, w);
	m_planes[1] = XMPlaneNormalize(m_planes[1]);

	// ����ü�� ���� ����� ����մϴ�.
	x = (float)(matrix._14 + matrix._11);
	y = (float)(matrix._24 + matrix._21);
	z = (float)(matrix._34 + matrix._31);
	w = (float)(matrix._44 + matrix._41);
	m_planes[2] = XMVectorSet(x, y, z, w);
	m_planes[2] = XMPlaneNormalize(m_planes[2]);

	// ����ü�� ������ ����� ����մϴ�.
	x = (float)(matrix._14 - matrix._11);
	y = (float)(matrix._24 - matrix._21);
	z = (float)(matrix._34 - matrix._31);
	w = (float)(matrix._44 - matrix._41);
	m_planes[3] = XMVectorSet(x, y, z, w);
	m_planes[3] = XMPlaneNormalize(m_planes[3]);

	// ����ü�� �� ����� ����մϴ�.
	x = (float)(matrix._14 - matrix._12);
	y = (float)(matrix._24 - matrix._22);
	z = (float)(matrix._34 - matrix._32);
	w = (float)(matrix._44 - matrix._42);
	m_planes[4] = XMVectorSet(x, y, z, w);
	m_planes[4] = XMPlaneNormalize(m_planes[4]);

	// ����ü�� �Ʒ� ����� ����մϴ�.
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

void Renderer::updateConstantBuffer()
{
	// ��� ���̴� Ÿ�Կ� ���� common constant buffer�� ������Ʈ��
	for (uint32 index = 0; index < CastValue<uint32>(ShaderType::Count); ++index)
	{
		auto& Shaders = ShaderManager->GetShaders(CastValue<ShaderType>(index));

		for (auto& Pair : Shaders)
		{
			std::shared_ptr<MShader>& Shader = Pair.second;
			auto &VS_CBuffers = Shader->GetVariableInfos();

			// PerConstant ���̾� 
			if (g_pRenderer->IsDirtyConstant())
			{
				Vec4 resolution = { CastValue<float>(g_pSetting->getResolutionWidth()), CastValue<float>(g_pSetting->getResolutionHeight()), 0.f, 0.f };
				BOOL bLight = TRUE;

				Shader->SetValue(TEXT("resolution"), resolution);
				Shader->SetValue(TEXT("bLight"), bLight);
				Shader->UpdateConstantBuffer(EConstantBufferLayer::Constant);
			}

			// PerTick ���̾�
			Shader->SetValue(TEXT("viewMatrix"), g_pMainGame->getMainCameraViewMatrix());
			Shader->SetValue(TEXT("projectionMatrix"), g_pMainGame->getMainCameraProjectioinMatrix());
			Shader->SetValue(TEXT("identityMatrix"), IDENTITYMATRIX);
			Shader->SetValue(TEXT("orthographicProjectionMatrix"), g_pMainGame->getMainCameraOrthographicProjectionMatrix());
			Shader->SetValue(TEXT("inverseOrthographicProjectionMatrix"), g_pMainGame->getMainCamera()->getInverseOrthographicProjectionMatrix());
			if (bool bDirectionalLightExist = CastValue<uint32>(_directionalLightDirection.size()) > 0)
			{
				std::vector<Vec4> lightPosition(3);
				std::vector<Mat4> lightViewProj(3);
				Test(lightViewProj, lightPosition);
				Shader->SetValue(TEXT("lightPos"), lightPosition);
				Shader->SetValue(TEXT("lightViewProjMatrix"), lightViewProj);
			}
			Shader->SetValue(TEXT("cascadeDistance"), _cascadeDistance);

			Shader->UpdateConstantBuffer(EConstantBufferLayer::PerTick);
		}
	}
	
	_bDirtyConstant = false;
	_directionalLightDirection.clear();
	// Costant���̾� CBuffer ������Ʈ
}

// DEPRECATED
//void Renderer::copyBufferData(std::vector<std::vector<FShaderVariable>> &infos, EConstantBufferLayer layer, uint32 index, const void *pData)
//{
//	if (infos[CastValue<uint32>(layer)].size() <= index)
//		return;
//
//	memcpy(infos[CastValue<uint32>(layer)][index].Value, pData, infos[CastValue<uint32>(layer)][index].Size);
//}

void Renderer::toggleRenderTarget()
{
	_drawRenderTarget = (true == _drawRenderTarget) ? false : true;
}

const bool Renderer::IsDirtyConstant() const
{
	return _bDirtyConstant;
}

void Renderer::Test(std::vector<Mat4>& lightViewProj, std::vector<Vec4>& lightPosition)
{
	float tanHalfVFov = tan(XMConvertToRadians(g_pSetting->getFov() / 2.f));
	float tanHalfHFov = tanHalfVFov * g_pSetting->getAspectRatio();

	for (int cascadeIndex = 0; cascadeIndex < CastValue<int>(EFrustumCascade::Far); ++cascadeIndex)
	{
		float depth = _cascadeDistance[cascadeIndex];
		float nextDepth = _cascadeDistance[cascadeIndex + 1];
		float xn = _cascadeDistance[cascadeIndex] * tanHalfHFov;
		float xf = _cascadeDistance[cascadeIndex + 1] * tanHalfHFov;
		float yn = _cascadeDistance[cascadeIndex] * tanHalfVFov;
		float yf = _cascadeDistance[cascadeIndex + 1] * tanHalfVFov;

		std::vector<Vec3> frustumCascadeVertices = {
			//near Face
			{xn,yn,depth},
			{-xn,yn,depth},
			{xn,-yn,depth},
			{-xn,-yn,depth},
			//far Face
			{xf,yf,nextDepth},
			{-xf,yf,nextDepth},
			{xf,-yf,nextDepth},
			{-xf,-yf,nextDepth}
		};

		XMMATRIX cameraWorldMatrix = XMLoadFloat4x4(&g_pMainGame->getMainCamera()->getInvesrViewMatrix());
		XMVECTOR cascadeCenter = XMVectorSet(VEC4ZERO.x, VEC4ZERO.y, VEC4ZERO.z, VEC4ZERO.w);
		for (auto& vertex : frustumCascadeVertices)
		{
			XMVECTOR worldVertex = XMVector3Transform(XMVectorSet(vertex.x, vertex.y, vertex.z, 1.f), cameraWorldMatrix);
			XMStoreFloat3(&vertex, worldVertex);

			cascadeCenter += worldVertex;
		}
		cascadeCenter /= CastValue<float>(frustumCascadeVertices.size());

		float maxDistance = 0.f;
		for (auto& vertex : frustumCascadeVertices)
		{
			Vec3 distance;
			XMStoreFloat3(&distance, XMVector3Length(XMLoadFloat3(&vertex) - cascadeCenter));

			maxDistance = std::max<float>(distance.x, maxDistance);
		}

		float radius = std::ceil(maxDistance * 16.f) / 16.f;

		XMVECTOR lightDirection = XMVector3Normalize(XMVectorSet(_directionalLightDirection[0].x, _directionalLightDirection[0].y, _directionalLightDirection[0].z, 0.f));
		XMVECTOR directionalLightPos = cascadeCenter - (lightDirection * radius);

		XMMATRIX LookAtMatrix = XMMatrixLookAtLH(directionalLightPos, cascadeCenter, XMLoadFloat3(&VEC3UP));
		XMMATRIX OrthograhpicMatrix = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, 0.f, 2.f * radius);
		
		XMStoreFloat4(&lightPosition[cascadeIndex], directionalLightPos);
		XMStoreFloat4x4(&lightViewProj[cascadeIndex], XMMatrixMultiply(LookAtMatrix, OrthograhpicMatrix));
		lightViewProj[cascadeIndex]._41 = round(lightViewProj[cascadeIndex]._41 * 10.f) / 10.f;
		lightViewProj[cascadeIndex]._42 = round(lightViewProj[cascadeIndex]._42 * 10.f) / 10.f;
	}
}
