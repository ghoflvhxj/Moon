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

using namespace DirectX;

void CombinePass::doPass(RenderQueue &renderQueue)
{
	// 렌더 큐 만들기
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		auto &VS_CBuffer_PerObject = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
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

	if (nullptr != primitiveData._pVertexBuffers[0])
	{
		primitiveData._pVertexBuffers[0]->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterials[0]->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
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

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffers[0]->getVertexCount(), 0);
}

void GeometryPass::doPass(RenderQueue &renderQueue)
{
	// 렌더 큐 만들기
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		if (primitiveData._primitiveType != EPrimitiveType::Mesh)
		{
			continue;
		}

		uint32 materialCount = static_cast<uint32>(primitiveData._pMaterials.size());
		for (uint32 materialIndex = 0; materialIndex < materialCount; ++materialIndex)
		{
			auto &VS_CBuffer_PerObject = primitiveData._pMaterials[materialIndex]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
			memcpy(VS_CBuffer_PerObject[0]._pValue, &primitive->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

			auto &PS_CBuffer_PerObject = primitiveData._pMaterials[materialIndex]->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
			BOOL bUseTexture = primitiveData._pMaterials[materialIndex]->useTextureType(TextureType::Normal) ? TRUE : FALSE;
			memcpy(PS_CBuffer_PerObject[0]._pValue, &bUseTexture, PS_CBuffer_PerObject[0]._size);
			bUseTexture = primitiveData._pMaterials[materialIndex]->useTextureType(TextureType::Specular) ? TRUE : FALSE;
			memcpy(PS_CBuffer_PerObject[1]._pValue, &bUseTexture, PS_CBuffer_PerObject[1]._size);
		}

		render(primitiveData);
	}
}

void GeometryPass::render(PrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	uint32 VertexBuffersCount = CastValue<uint32>(primitiveData._pVertexBuffers.size());
	for (uint32 index = 0; index< VertexBuffersCount; ++index)
	{
		auto &pVertexBuffer = primitiveData._pVertexBuffers[index];
		FALSE_CHECK_ASSERT_MSG(pVertexBuffer, "버텍스 버퍼가 nullptr이면 안됩니다")
		pVertexBuffer->setBufferToDevice(stride, offset);

		//if (nullptr != primitiveData._pIndexBuffer)
		//{
		//	primitiveData._pIndexBuffer->setBufferToDevice(offset);
		//}

		uint32 matrialIndex = CastValue<uint32>(primitiveData._geometryMaterialLinkIndex[index]);
		auto &pMaterial = primitiveData._pMaterials[matrialIndex];
		FALSE_CHECK_ASSERT_MSG(pVertexBuffer, "매터리얼이 nullptr이면 안됩니다")

		g_pGraphicDevice->getContext()->IASetPrimitiveTopology(pMaterial->getTopology());

		//---------------------------------------------------------------------------------------------------------------------------------
		// Vertex Shader
		auto &variableInfosVS = pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
		primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
		primitiveData._pVertexShader->SetToDevice();

		//---------------------------------------------------------------------------------------------------------------------------------
		// Pixel Shader
		auto &variableInfosPS = pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
		primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
		primitiveData._pPixelShader->SetToDevice();

		//auto &variableInfosPS2 = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
		//primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS2);
		//primitiveData._pPixelShader->SetToDevice();

		pMaterial->SetTexturesToDevice();

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

		g_pGraphicDevice->getContext()->Draw(pVertexBuffer->getVertexCount(), 0);
	}
}

void LightPass::doPass(RenderQueue &renderQueue)
{
	// 렌더 큐 만들기
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		if (primitiveData._primitiveType != EPrimitiveType::Light)
		{
			continue;
		}

		auto &VS_CBuffer_PerObject = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
		memcpy(VS_CBuffer_PerObject[0]._pValue, &primitive->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);
		
		Vec3 trans = primitive->getTranslation();
		Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
		Vec4 color = { 1.f, 1.f, 1.f, 1.f };
		auto &PS_CBuffer_PerObject = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
		memcpy(PS_CBuffer_PerObject[0]._pValue, &transAndRange, PS_CBuffer_PerObject[0]._size);
		XMVECTOR rotationVector = XMLoadFloat3(&primitive->getRotation());
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
		Mat4 rotMatrix = IDENTITYMATRIX;
		XMStoreFloat4x4(&rotMatrix, rotationMatrix);
		Vec3 look = { rotMatrix._31, rotMatrix._32, rotMatrix._33 };
		memcpy(PS_CBuffer_PerObject[1]._pValue, &look, PS_CBuffer_PerObject[1]._size);
		memcpy(PS_CBuffer_PerObject[2]._pValue, &color, PS_CBuffer_PerObject[2]._size);
		memcpy(PS_CBuffer_PerObject[3]._pValue, &g_pMainGame->getMainCamera()->getInvesrViewMatrix(), PS_CBuffer_PerObject[3]._size);
		memcpy(PS_CBuffer_PerObject[4]._pValue, &g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix(), PS_CBuffer_PerObject[4]._size);

		render(primitiveData);
	}
}

void LightPass::render(PrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffers[0])
	{
		primitiveData._pVertexBuffers[0]->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterials[0]->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pPixelShader->SetToDevice();

	//primitiveData._pMaterial->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Disable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffers[0]->getVertexCount(), 0);
}

void SkyPass::doPass(RenderQueue & renderQueue)
{
	// 렌더 큐 만들기
	for (auto& primitive : renderQueue)
	{
		PrimitiveData primitiveData = {};
		primitive->getPrimitiveData(primitiveData);

		if (primitiveData._primitiveType != EPrimitiveType::Sky)
		{
			continue;
		}

		auto &VS_CBuffer_PerObject = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
		memcpy(VS_CBuffer_PerObject[0]._pValue, &primitive->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

		//auto &PS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
		//BOOL bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Normal) ? TRUE : FALSE;
		//memcpy(PS_CBuffer_PerObject[0]._pValue, &bUseTexture, PS_CBuffer_PerObject[0]._size);
		//bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Specular) ? TRUE : FALSE;
		//memcpy(PS_CBuffer_PerObject[1]._pValue, &bUseTexture, PS_CBuffer_PerObject[1]._size);

		render(primitiveData);
	}
}

void SkyPass::render(PrimitiveData & primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffers[0])
	{
		primitiveData._pVertexBuffers[0]->setBufferToDevice(stride, offset);
	}

	if (nullptr != primitiveData._pIndexBuffer)
	{
		primitiveData._pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterials[0]->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pVertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pVertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterials[0]->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pPixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pPixelShader->SetToDevice();

	primitiveData._pMaterials[0]->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::None));

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

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffers[0]->getVertexCount(), 0);
}
