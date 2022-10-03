#include "stdafx.h"
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

using namespace DirectX;

Renderer::Renderer(void) noexcept
	: _drawRenderTarget{ true }
	, _bDirtyConstant{ true }
{
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

	_pMeshComponent = std::make_shared<StaticMeshComponent>("Base/Plane.fbx");
	_pMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 1.f });
	_pMeshComponent->setScale(Vec3{ CastValue<float>(g_pSetting->getResolutionWidth()), CastValue<float>(g_pSetting->getResolutionHeight()), 1.f });
	_pMeshComponent->SceneComponent::Update(0.f);

	for (int i = 0; i < CastValue<int>(ERenderTarget::Count); ++i)
	{
		_renderTargets.emplace_back(std::make_shared<RenderTarget>());
	}

	_renderPasses.emplace_back(CreateRenderPass<ShadowDepthPass>());
	{
		_renderPasses.back()->SetUseOwningDepthStencilBuffer(true);
		_renderPasses[enumToIndex(ERenderPass::ShadowDepth)]->initializeRenderTargets(_renderTargets,
			ERenderTarget::ShadowDepth
		);

		_renderPasses[enumToIndex(ERenderPass::ShadowDepth)]->setShader(TEXT("ShadowDepth.cso"), TEXT("ShadowDepthPixel.cso"));
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
}

void Renderer::addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> &pComponent)
{
	_primitiveComponents.push_back(pComponent);
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

	totalPrimitiveCount = CastValue<uint32>(_primitiveComponents.size());
	FrustumCulling();

	updateConstantBuffer();
	
	// 기본 패스
	uint32 combinePassIndex = CastValue<uint32>(ERenderPass::Combine);
	for (uint32 i = 0; i < combinePassIndex; ++i)
	{
		_renderPasses[i]->begin();
		_renderPasses[i]->doPass(_primitiveComponents);
		_renderPasses[i]->end();
	}

	// 혼합 패스
	std::vector<std::shared_ptr<PrimitiveComponent>> temp;
	temp.emplace_back(_pMeshComponent);
	_renderPasses[combinePassIndex]->begin();
	_renderPasses[combinePassIndex]->doPass(temp);
	_renderPasses[combinePassIndex]->end();

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
	_primitiveComponents.clear();

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

	std::vector<std::shared_ptr<PrimitiveComponent>> culledPrimitiveComponents;
	culledPrimitiveComponents.reserve(_primitiveComponents.size());

	for (auto &primitive : _primitiveComponents)
	{
		std::shared_ptr<BoundingBox> boundingBox = nullptr;
		if (false == primitive->getBoundingBox(boundingBox))
		{
			culledPrimitiveComponents.emplace_back(primitive);
			continue;
		}

		if (boundingBox->cullSphere(m_planes, primitive->getWorldTranslation(), boundingBox->getLength(primitive->getScale())/2.f))
		{
			culledPrimitiveComponents.emplace_back(primitive);
		}
	}

	_primitiveComponents = std::move(culledPrimitiveComponents);

	showPrimitiveCount = CastValue<uint32>(_primitiveComponents.size());
	culledPrimitiveCount = totalPrimitiveCount - showPrimitiveCount;
}

void Renderer::updateConstantBuffer()
{
	for (uint32 index = 0; index < CastValue<uint32>(ShaderType::Count); ++index)
	{
		auto& shaders = g_pShaderManager->getShaders(CastValue<ShaderType>(index));

		for (auto& pair : shaders)
		{
			std::shared_ptr<Shader> shader = pair.second;
			auto &VS_CBuffers = shader->getVariableInfos();

			// PerConstant 레이어 
			if (g_pRenderer->IsDirtyConstant())
			{
				int resoultionWidth = g_pSetting->getResolutionWidth();
				int resoultionHeight = g_pSetting->getResolutionHeight();
				BOOL bLight = TRUE;

				copyBufferData(VS_CBuffers, ConstantBuffersLayer::Constant, 0, &resoultionWidth);
				copyBufferData(VS_CBuffers, ConstantBuffersLayer::Constant, 1, &resoultionHeight);
				copyBufferData(VS_CBuffers, ConstantBuffersLayer::Constant, 2, &bLight);
				shader->UpdateConstantBuffer(ConstantBuffersLayer::Constant, VS_CBuffers[CastValue<uint32>(ConstantBuffersLayer::Constant)]);
			}

			// PerTick 레이어
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 0, &g_pMainGame->getMainCameraViewMatrix());
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 1, &g_pMainGame->getMainCameraProjectioinMatrix());
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 2, &IDENTITYMATRIX);
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 3, &g_pMainGame->getMainCameraOrthographicProjectionMatrix());
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 4, &g_pMainGame->getMainCamera()->getInverseOrthographicProjectionMatrix());

			uint32 directionalLightCount = CastValue<uint32>(_directionalLightDirection.size());
			if (directionalLightCount > 0)
			{
				Mat4 view, proj;
				Test(view, proj);
				copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 5, &view);
				copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 6, &proj);
			}

			shader->UpdateConstantBuffer(ConstantBuffersLayer::PerTick, VS_CBuffers[CastValue<uint32>(ConstantBuffersLayer::PerTick)]);
		}
	}
	
	_bDirtyConstant = false;
	_directionalLightDirection.clear();
	// Costant레이어 CBuffer 업데이트
}

void Renderer::copyBufferData(std::vector<std::vector<VariableInfo>> &infos, ConstantBuffersLayer layer, uint32 index, const void *pData)
{
	if (infos[CastValue<uint32>(layer)].size() <= index)
		return;

	memcpy(infos[CastValue<uint32>(layer)][index]._pValue, pData, infos[CastValue<uint32>(layer)][index]._size);
}

void Renderer::toggleRenderTarget()
{
	_drawRenderTarget = (true == _drawRenderTarget) ? false : true;
}

const bool Renderer::IsDirtyConstant() const
{
	return _bDirtyConstant;
}

void Renderer::Test(Mat4 &viewMatrix, Mat4 &orthographicMatrix)
{
	enum class EFrustumCascade
	{
		Near,
		Middle,
		Far,
		Count
	};
	
	std::vector<float> frustumCascadeZ(3, 0);
	frustumCascadeZ[CastValue<int>(EFrustumCascade::Near)]	= 0.1f;
	frustumCascadeZ[CastValue<int>(EFrustumCascade::Middle)] = 6.f;
	frustumCascadeZ[CastValue<int>(EFrustumCascade::Far)]	= 1000.f;

	float tanHalfFov = tan(g_pSetting->getFov() / 2.f);
	float tanHalfAspectRatio = tan(g_pSetting->getAspectRatio() / 2.f);

	//for (int cascadeIndex = 0; cascadeIndex < CastValue<int>(EFrustumCascade::Count) - 1; ++cascadeIndex)
	for (int cascadeIndex = 0; cascadeIndex < 1; ++cascadeIndex)

	{
		float depth = frustumCascadeZ[cascadeIndex];
		float width = tanHalfFov * depth;
		float hegiht = tanHalfAspectRatio * depth;

		float nextDepth = frustumCascadeZ[cascadeIndex + 1];
		float nextWidth = tanHalfFov * nextDepth;
		float nextHegiht = tanHalfAspectRatio * nextDepth;

		std::vector<Vec4> frustumCascadeVertices = {
			{ -width, hegiht, depth, 1.f },
			{ width, hegiht, depth, 1.f },
			{ -width,-hegiht, depth, 1.f },
			{ width,-hegiht, depth, 1.f },
			{ -nextWidth, nextHegiht, nextDepth, 1.f },
			{ nextWidth, nextHegiht, nextDepth, 1.f },
			{ -nextWidth,-nextHegiht, nextDepth, 1.f },
			{ nextWidth,-nextHegiht, nextDepth, 1.f },
		};

		Mat4 cameraWorldMatrix = g_pMainGame->getMainCamera()->getInvesrViewMatrix();
		XMVECTOR cascadeCenter = XMVectorSet(VEC4ZERO.x, VEC4ZERO.y, VEC4ZERO.z, VEC4ZERO.w);
		for (auto& vertex : frustumCascadeVertices)
		{
			XMVECTOR worldVertex = XMVector3TransformCoord(XMVectorSet(vertex.x, vertex.y, vertex.z, vertex.w), XMLoadFloat4x4(&cameraWorldMatrix));
			XMStoreFloat4(&vertex, worldVertex);

			cascadeCenter += worldVertex;
		}

		cascadeCenter /= 8.f;
		float maxDistance = 0.f;
		for (auto& vertex : frustumCascadeVertices)
		{
			Vec4 distance;
			XMStoreFloat4(&distance, XMVector4Length(XMLoadFloat4(&vertex) - cascadeCenter));

			maxDistance = std::max<float>(distance.x, maxDistance);
		}

		float radius = std::ceil(maxDistance * 16.0f) / 16.0f;

		XMVECTOR lightDirection = XMVector3Normalize(XMLoadFloat3(&_directionalLightDirection[0]));
		XMVECTOR directionalLightPos = cascadeCenter - (lightDirection * radius);

		XMMATRIX LookAtMatrix = XMMatrixLookAtLH(directionalLightPos, cascadeCenter, XMLoadFloat3(&VEC3UP));
		XMMATRIX OrthograhpicMatrix = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, 0.f, 2.f * radius);

		XMStoreFloat4x4(&viewMatrix, LookAtMatrix);
		XMStoreFloat4x4(&orthographicMatrix, OrthograhpicMatrix);
	}
}
