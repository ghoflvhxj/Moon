#include "Include.h"
#include "CombinePass.h"

// Renderer
#include "Renderer.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

// Game
#include "MainGame.h"

// Actor
#include "Camera.h"

using namespace DirectX;

const bool CombinePass::processPrimitiveData(const FPrimitiveData &primitiveData)
{
	PrimitiveComponent* PrimitiveComponent = primitiveData._pPrimitive.lock().get();

	auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	memcpy(VS_CBuffer_PerObject[0]._pValue, &PrimitiveComponent->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

	return true;
}

void CombinePass::render(const FPrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != primitiveData._pIndexBuffer)
	//{
	//	primitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	_vertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	_vertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	_pixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	_pixelShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Disable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}

const bool GeometryPass::processPrimitiveData(const FPrimitiveData &primitiveData)
{
	switch (primitiveData._primitiveType)
	{
	case EPrimitiveType::Mesh:
		break;
	case EPrimitiveType::Collision:
		if (g_pRenderer->IsDrawCollision() == false)
		{
			return false;
		}
		break;
	default:
		return false;
	}

	PrimitiveComponent* PrimitiveComponent = primitiveData._pPrimitive.lock().get();

	auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	memcpy(VS_CBuffer_PerObject[0]._pValue, &PrimitiveComponent->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

	BOOL animated = primitiveData._matrices != nullptr;
	if (animated)
	{
		memcpy(VS_CBuffer_PerObject[1]._pValue, primitiveData._matrices, VS_CBuffer_PerObject[1]._size);
	}
	memcpy(VS_CBuffer_PerObject[2]._pValue, &animated, VS_CBuffer_PerObject[2]._size);

	auto &PS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	if (PS_CBuffer_PerObject.empty() == false)
	{
		BOOL bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Normal) ? TRUE : FALSE;
		memcpy(PS_CBuffer_PerObject[0]._pValue, &bUseTexture, PS_CBuffer_PerObject[0]._size);
		bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Specular) ? TRUE : FALSE;
		memcpy(PS_CBuffer_PerObject[1]._pValue, &bUseTexture, PS_CBuffer_PerObject[1]._size);
	}

	return true;
}

void GeometryPass::render(const FPrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != primitiveData._pIndexBuffer)
	//{
	//	primitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pMaterial->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pMaterial->getPixelShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);

	primitiveData._pMaterial->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(primitiveData._pMaterial->getFillMode(), primitiveData._pMaterial->getCullMode()));

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

const bool ShadowDepthPass::processPrimitiveData(const FPrimitiveData & primitiveData)
{
	// ·»´õ Å¥ ¸¸µé±â
	if (primitiveData._primitiveType != EPrimitiveType::Mesh)
	{
		return false;
	}

	return true;
}

void ShadowDepthPass::render(const FPrimitiveData & primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != primitiveData._pIndexBuffer)
	//{
	//	primitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	_vertexShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	_vertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	_pixelShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	_pixelShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	auto &variableInfosGS = _geometryShader->getVariableInfos()[CastValue<int>(ShaderType::Geometry)];
	_geometryShader->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosGS);
	_geometryShader->SetToDevice();

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
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(primitiveData._pIndexBuffer-CO>getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}

const bool LightPass::processPrimitiveData(const FPrimitiveData &primitiveData)
{
	// ·»´õ Å¥ ¸¸µé±â
	if (primitiveData._primitiveType != EPrimitiveType::Light)
	{
		return false;
	}

	PrimitiveComponent* PrimitiveComponent = primitiveData._pPrimitive.lock().get();

	auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	memcpy(VS_CBuffer_PerObject[0]._pValue, &PrimitiveComponent->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

	Vec3 trans = PrimitiveComponent->getTranslation();
	Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
	Vec4 color = { 1.f, 1.f, 1.f, 1.f };
	auto &PS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	memcpy(PS_CBuffer_PerObject[0]._pValue, &transAndRange, PS_CBuffer_PerObject[0]._size);
	XMVECTOR rotationVector = XMLoadFloat3(&PrimitiveComponent->getRotation());
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
	Mat4 rotMatrix = IDENTITYMATRIX;
	XMStoreFloat4x4(&rotMatrix, rotationMatrix);
	Vec3 look = { rotMatrix._31, rotMatrix._32, rotMatrix._33 };
	memcpy(PS_CBuffer_PerObject[1]._pValue, &look, PS_CBuffer_PerObject[1]._size);
	memcpy(PS_CBuffer_PerObject[2]._pValue, &color, PS_CBuffer_PerObject[2]._size);
	memcpy(PS_CBuffer_PerObject[3]._pValue, &g_pMainGame->getMainCamera()->getViewMatrix(), PS_CBuffer_PerObject[3]._size);
	memcpy(PS_CBuffer_PerObject[4]._pValue, &g_pMainGame->getMainCamera()->getInvesrViewMatrix(), PS_CBuffer_PerObject[4]._size);
	memcpy(PS_CBuffer_PerObject[5]._pValue, &g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix(), PS_CBuffer_PerObject[5]._size);

	return true;
}

void LightPass::render(const FPrimitiveData &primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != primitiveData._pIndexBuffer)
	//{
	//	primitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pMaterial->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pMaterial->getPixelShader()->SetToDevice();

	//primitiveData._pMaterial->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);

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

	g_pGraphicDevice->getContext()->Draw(primitiveData._pVertexBuffer->getVertexCount(), 0);
}

const bool SkyPass::processPrimitiveData(const FPrimitiveData &primitiveData)
{
	if (primitiveData._primitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	PrimitiveComponent* PrimitiveComponent = primitiveData._pPrimitive.lock().get();

	auto &VS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	memcpy(VS_CBuffer_PerObject[0]._pValue, &PrimitiveComponent->getWorldMatrix(), VS_CBuffer_PerObject[0]._size);

	//auto &PS_CBuffer_PerObject = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	//BOOL bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Normal) ? TRUE : FALSE;
	//memcpy(PS_CBuffer_PerObject[0]._pValue, &bUseTexture, PS_CBuffer_PerObject[0]._size);
	//bUseTexture = primitiveData._pMaterial->useTextureType(TextureType::Specular) ? TRUE : FALSE;
	//memcpy(PS_CBuffer_PerObject[1]._pValue, &bUseTexture, PS_CBuffer_PerObject[1]._size);

	return true;
}

void SkyPass::render(const FPrimitiveData & primitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != primitiveData._pVertexBuffer)
	{
		primitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != primitiveData._pIndexBuffer)
	//{
	//	primitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(primitiveData._pMaterial->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, ConstantBuffersLayer::PerObject);
	primitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosVS);
	primitiveData._pMaterial->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, ConstantBuffersLayer::PerObject);
	primitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(ConstantBuffersLayer::PerObject, variableInfosPS);
	primitiveData._pMaterial->getPixelShader()->SetToDevice();

	primitiveData._pMaterial->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Frontface));

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
