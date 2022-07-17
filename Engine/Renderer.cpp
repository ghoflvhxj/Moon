#include "stdafx.h"
#include "Renderer.h"

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
}

void Renderer::initialize(void) noexcept
{
	_pMeshComponent = std::make_shared<StaticMeshComponent>("Base/Box.fbx");
	_pMeshComponent->getStaticMesh()->getMaterial(0)->setShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));
	_pMeshComponent->setTranslation(Vec3{ 0.f, 0.f, 15.f });
	_pMeshComponent->setScale(Vec3{ CastValue<float>(g_pSetting->getResolutionWidth()), CastValue<float>(g_pSetting->getResolutionHeight()), 1.f });
	_pMeshComponent->SceneComponent::Update(0.f);

	for (int i = 0; i < CastValue<int>(ERenderTarget::Count); ++i)
	{
		_renderTargets.emplace_back(std::make_shared<RenderTarget>());
	}

	/*
	_pMeshComponent->getStaticMesh()->getMaterial(0)->setDepthWriteMode(Graphic::DepthWriteMode::Disable);
	_pMeshComponent->getStaticMesh()->getMaterial(0)->setShader(TEXT("TexVertexShader.cso"), TEXT("DeferredShader.cso"));

	addRenderPass(TEXT("DepthPrePass"));
	{
		std::vector<std::shared_ptr<RenderTarget>> rt = {
			_renderTargetMap[TEXT("DepthPre")],
		};
		_renderPassMap[TEXT("DepthPrePass")]->initializeRenderTarget(rt);
		_renderPassMap[TEXT("DepthPrePass")]->setShader(TEXT("DepthPre.cso"), TEXT(""));
	}

	addRenderPass(TEXT("GeometryPass"));
	{
		std::vector<std::shared_ptr<RenderTarget>> rt = {
			_renderTargetMap[TEXT("Albedo")],
			_renderTargetMap[TEXT("Depth")],
			_renderTargetMap[TEXT("Normal")],
			_renderTargetMap[TEXT("Specular")]
		};
		_renderPassMap[TEXT("GeometryPass")]->initializeRenderTarget(rt);
	}

	addRenderPass(TEXT("LightPass"));
	{
		std::vector<std::shared_ptr<RenderTarget>> rt = {
			_renderTargetMap[TEXT("LightDiffuse")],
			_renderTargetMap[TEXT("LightSpecular")],
		};

		std::vector<std::shared_ptr<TextureComponent>> textureList = {
			_renderTargetMap[TEXT("Depth")]->getRenderTargetTexture(),
			_renderTargetMap[TEXT("Normal")]->getRenderTargetTexture(),
			_renderTargetMap[TEXT("Specular")]->getRenderTargetTexture()
		};

		_renderPassMap[TEXT("LightPass")]->initializeRenderTarget(rt);
		_renderPassMap[TEXT("LightPass")]->initializeTexture(textureList);
	}

	addRenderPass(TEXT("CombinePass"));
	{
		std::vector<std::shared_ptr<TextureComponent>> textureList = {
			_renderTargetMap[TEXT("Albedo")]->getRenderTargetTexture(),
			_renderTargetMap[TEXT("LightDiffuse")]->getRenderTargetTexture(),
			_renderTargetMap[TEXT("LightSpecular")]->getRenderTargetTexture()
		};

		_renderPasses[TEXT("CombinePass")]->initializeTexture(textureList);
	}
	*/

	_renderPasses.emplace_back(CreateRenderPass<GeometryPass>());
	{
		RenderTargets rt = {
			_renderTargets[enumToIndex(ERenderTarget::Albedo)],
			_renderTargets[enumToIndex(ERenderTarget::Depth)],
			_renderTargets[enumToIndex(ERenderTarget::Normal)],
			_renderTargets[enumToIndex(ERenderTarget::Specular)]
		};
		_renderPasses[enumToIndex(ERenderPass::Geometry)]->initializeRenderTarget(rt);
	}

	_renderPasses.emplace_back(CreateRenderPass<LightPass>());
	{
		std::vector<std::shared_ptr<RenderTarget>> rt = {
			_renderTargets[enumToIndex(ERenderTarget::LightDiffuse)],
			_renderTargets[enumToIndex(ERenderTarget::LightSpecular)],
		};

		std::vector<std::shared_ptr<RenderTarget>> textureList = {
			_renderTargets[enumToIndex(ERenderTarget::Depth)],
			_renderTargets[enumToIndex(ERenderTarget::Normal)],
			_renderTargets[enumToIndex(ERenderTarget::Specular)]
		};

		_renderPasses[enumToIndex(ERenderPass::Light)]->initializeRenderTarget(rt);
		_renderPasses[enumToIndex(ERenderPass::Light)]->initializeResourceViewByRenderTarget(textureList);
	}

	_renderPasses.emplace_back(CreateRenderPass<CombinePass>());
	{
		std::vector<std::shared_ptr<RenderTarget>> textureList = {
			_renderTargets[enumToIndex(ERenderTarget::Albedo)],
			_renderTargets[enumToIndex(ERenderTarget::LightDiffuse)],
			_renderTargets[enumToIndex(ERenderTarget::LightSpecular)]
		};
		_renderPasses[enumToIndex(ERenderPass::Combine)]->setShader(TEXT("Deferred.cso"), TEXT("DeferredShader.cso"));
		_renderPasses[enumToIndex(ERenderPass::Combine)]->initializeResourceViewByRenderTarget(textureList);
	}
}

void Renderer::addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> pComponent)
{
	_primitiveComponents.push_back(pComponent);
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

void Renderer::render(void)
{
	g_pGraphicDevice->Begin();

	std::vector<std::shared_ptr<PrimitiveComponent>> primitiveComponents(std::move(_primitiveComponents));
	//FrustumCulling(primitiveComponents);

	updateConstantBuffer();

	// 기본 패스
	uint32 combinePassIndex = CastValue<uint32>(ERenderPass::Combine);
	for (uint32 i = 0; i < combinePassIndex; ++i)
	{
		_renderPasses[i]->begin();
		_renderPasses[i]->doPass(primitiveComponents);
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

	g_pGraphicDevice->End();
}

void Renderer::updateConstantBuffer()
{
	bool bUpdateConstantLayer = !g_pRenderer->IsDirtyConstant();

	for (uint32 index = 0; index < CastValue<uint32>(ShaderType::Count); ++index)
	{
		auto& shaders = g_pShaderManager->getShaders(CastValue<ShaderType>(index));

		for (auto& pair : shaders)
		{
			std::shared_ptr<Shader> shader = pair.second;
			auto &VS_CBuffers = shader->getVariableInfo();

			// PerConstant 레이어
			if (bUpdateConstantLayer)
			{
				int resoultionWidth = g_pSetting->getResolutionWidth();
				int resoultionHeight = g_pSetting->getResolutionHeight();

				copyBufferData(VS_CBuffers, ConstantBuffersLayer::Constant, 0, &resoultionWidth);
				copyBufferData(VS_CBuffers, ConstantBuffersLayer::Constant, 1, &resoultionHeight);
				shader->UpdateConstantBuffer(ConstantBuffersLayer::Constant, VS_CBuffers[CastValue<uint32>(ConstantBuffersLayer::Constant)]);
			}

			// PerTick 레이어
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 0, &g_pMainGame->getMainCameraViewMatrix());
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 1, &g_pMainGame->getMainCameraProjectioinMatrix());
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 2, &IDENTITYMATRIX);
			copyBufferData(VS_CBuffers, ConstantBuffersLayer::PerTick, 3, &g_pMainGame->getMainCameraOrthographicProjectionMatrix());
			shader->UpdateConstantBuffer(ConstantBuffersLayer::PerTick, VS_CBuffers[CastValue<uint32>(ConstantBuffersLayer::PerTick)]);
		}
	}
	
	_bDirtyConstant = false;
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

void Renderer::renderMesh()
{


	// 메시 렌더링 흐름
	// 1. 매터리얼에 설정된 쉐이더를 사용하는 것
	// 2. 렌더 패스에 설정된 쉐이더를 사용하는 것

	// 메시 렌더링 
	// 1. 스태틱 메시
	// 2. 다이나믹 메시


}

void Renderer::test(PrimitiveData &renderData)
{
	////---------------------------------------------------------------------------------------------------------------------------------
	//// Input Assembler
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;

	//if (nullptr != renderData._pVertexBuffer)
	//{
	//	renderData._pVertexBuffer->setBufferToDevice(stride, offset);
	//}

	//if (nullptr != renderData._pIndexBuffer)
	//{
	//	renderData._pIndexBuffer->setBufferToDevice(offset);
	//}

	//g_pGraphicDevice->getContext()->IASetPrimitiveTopology(renderData._pMaterial->getTopology());

	////---------------------------------------------------------------------------------------------------------------------------------
	//// Vertex Shader
	//auto &variableInfos = renderData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, CastValue<uint32>(ConstantBuffersLayer::PerObject));
	//renderData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfos);
	//renderData._pVertexShader->SetToDevice();

	////---------------------------------------------------------------------------------------------------------------------------------
	//// Pixel Shader
	//auto &variableInfos = renderData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, CastValue<uint32>(ConstantBuffersLayer::PerObject));
	//renderData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfos);
	//renderData._pVertexShader->SetToDevice();

	////---------------------------------------------------------------------------------------------------------------------------------
	//// RasterizerState

	////--------------------------------------------------------------------------------------------------------------------------------
	//// DepthStencilState

	////--------------------------------------------------------------------------------------------------------------------------------
	//// OutputMerge

	////--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(renderData._pIndexBuffer->getIndexCount(), 0, 0);
}

const bool Renderer::IsDirtyConstant() const
{
	return _bDirtyConstant;
}

//g_pShaderManager->getVertexShader();
//g_pGraphicDevice->getContext()->VSSetShader(_vertexShader, nullptr, 0);
//{
//	VertexShaderConstantBuffer data = {
//		pComponent->getWorldMatrix(),
//		g_pMainGame->getMainCameraViewMatrix(),
//		g_pMainGame->getMainCameraProjectioinMatrix()
//	};

//	if (PrimitiveComponent::RenderMode::Orthogonal == pComponent->getRenderMdoe())
//	{
//		data._cameraViewMatrix = IDENTITYMATRIX;
//		data._projectionMatrix = g_pMainGame->getMainCameraOrthographicProjectionMatrix();
//	}

//	auto pConstantBuffer = _pVertexConstantBuffer->getBuffer();
//	_pVertexConstantBuffer->update(&data, sizeof(VertexShaderConstantBuffer));

//	g_pGraphicDevice->getContext()->VSSetConstantBuffers(0u, 1u, &pConstantBuffer);
//}
