#include "stdafx.h"
#include "CombinePass.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

// Game
#include "MainGame.h"

// Actor
#include "Camera.h"

CombinePass::CombinePass()
	: RenderPass()
{

}

CombinePass::~CombinePass()
{

}

void CombinePass::doPass(RenderQueue &renderQueue)
{
	// ·»´õ Å¥ ¸¸µé±â
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
		memcpy(VS_CBuffer_PerObject[0]._pValue, &primitive->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

		render(primitiveData);
	}
}

void CombinePass::render(PrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pPixelShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Disable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}

GeometryPass::GeometryPass()
{

}

GeometryPass::~GeometryPass()
{

}

void GeometryPass::doPass(RenderQueue &renderQueue)
{
	// ·»´õ Å¥ ¸¸µé±â
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		if (primitiveData._primitiveType != EPrimitiveType::Mesh)
		{
			continue;
		}

		auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
		memcpy(VS_CBuffer_PerObject[0]._pValue, &primitive->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

		auto &PS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerTick);
		bool bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Normal);
		memcpy(PS_CBuffer_PerObject[0]._pValue, &bUseTexture, PS_CBuffer_PerObject[0]._size);
		bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Specular);
		memcpy(PS_CBuffer_PerObject[1]._pValue, &bUseTexture, PS_CBuffer_PerObject[1]._size);

		render(primitiveData);
	}
}

void GeometryPass::render(PrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pPixelShader->SetToDevice();

	primitiveData._pMaterial->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Enable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}

void LightPass::doPass(RenderQueue &renderQueue)
{
	// ·»´õ Å¥ ¸¸µé±â
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		if (primitiveData._primitiveType != EPrimitiveType::Light)
		{
			continue;
		}

		auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
		memcpy(VS_CBuffer_PerObject[0]._pValue, &primitive->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);
		
		Vec3 trans = primitive->getTranslation();
		Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
		Vec4 color = { 1.f, 1.f, 1.f, 1.f };
		auto &PS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
		memcpy(PS_CBuffer_PerObject[0]._pValue, &transAndRange, PS_CBuffer_PerObject[0]._size);
		memcpy(PS_CBuffer_PerObject[1]._pValue, &color, PS_CBuffer_PerObject[1]._size);
		memcpy(PS_CBuffer_PerObject[2]._pValue, &g_pMainGame->getMainCamera()->getInvesrViewMatrix(), PS_CBuffer_PerObject[2]._size);
		memcpy(PS_CBuffer_PerObject[3]._pValue, &g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix(), PS_CBuffer_PerObject[3]._size);

		render(primitiveData);
	}
}

void LightPass::render(PrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pPixelShader->SetToDevice();

	//primitiveData._pMaterial->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Enable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}
