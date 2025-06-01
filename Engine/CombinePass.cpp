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
	auto &variablesVS = primitiveData._pMaterial->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	_vertexShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variablesVS);	// RenderPass에 설정된 쉐이더를 사용함!
	_vertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariables(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	_pixelShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS); // RenderPass에 설정된 쉐이더를 사용함!
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

	return RenderPass::processPrimitiveData(primitiveData);
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
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	primitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	primitiveData._pMaterial->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariables(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	primitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
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
	// 렌더 큐 만들기
	if (primitiveData._primitiveType != EPrimitiveType::Mesh)
	{
		return false;
	}

	return RenderPass::processPrimitiveData(primitiveData);
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
	auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	_vertexShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	_vertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariables(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	_pixelShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
	_pixelShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	// 주석 제거하기
	auto &variableInfosGS = _geometryShader->GetVariables()[CastValue<int>(ShaderType::Geometry)];
	_geometryShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosGS);
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
	// 렌더 큐 만들기
	if (primitiveData._primitiveType != EPrimitiveType::Light)
	{
		return false;
	}

	PrimitiveComponent* PrimitiveComponent = primitiveData._pPrimitive.lock().get();

	Vec3 trans = PrimitiveComponent->getTranslation();
	Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
	Vec4 color = { 1.f, 1.f, 1.f, 1.f };

	XMVECTOR rotationVector = XMLoadFloat3(&PrimitiveComponent->getRotation());
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
	Mat4 rotMatrix = IDENTITYMATRIX;
	XMStoreFloat4x4(&rotMatrix, rotationMatrix);
	Vec3 look = { rotMatrix._31, rotMatrix._32, rotMatrix._33 };

	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("g_lightPosition"), transAndRange);
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("g_lightDirection"), look);
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("g_lightColor"), color);
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("g_inverseCameraViewMatrix"), g_pMainGame->getMainCamera()->getInvesrViewMatrix());
	primitiveData._pMaterial->getPixelShader()->SetValue(TEXT("g_inverseProjectiveMatrix"), g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix());

	return RenderPass::processPrimitiveData(primitiveData);
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
	//auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	//primitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	primitiveData._pMaterial->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	//auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	//primitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
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

const bool SkyPass::processPrimitiveData(const FPrimitiveData& primitiveData)
{
	if (primitiveData._primitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	return RenderPass::processPrimitiveData(primitiveData);
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
	//auto &variableInfosVS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	//primitiveData._pMaterial->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	primitiveData._pMaterial->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	//auto &variableInfosPS = primitiveData._pMaterial->getConstantBufferVariableInfos(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	//primitiveData._pMaterial->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
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
