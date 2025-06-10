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

// 임시
#include "LightComponent.h"

using namespace DirectX;

void CombinePass::render(const FPrimitiveData &PrimitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != PrimitiveData._pVertexBuffer)
	{
		PrimitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != PrimitiveData._pIndexBuffer)
	//{
	//	PrimitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(PrimitiveData._pMaterial.lock()->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variablesVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	_vertexShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variablesVS);	// RenderPass에 설정된 쉐이더를 사용함!
	_vertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Pixel, EConstantBufferLayer::PerObject);
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
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(PrimitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(PrimitiveData._pVertexBuffer->getVertexCount(), 0);
}

const bool GeometryPass::processPrimitiveData(const FPrimitiveData &PrimitiveData)
{
	switch (PrimitiveData._primitiveType)
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

	return RenderPass::processPrimitiveData(PrimitiveData);
}

void GeometryPass::render(const FPrimitiveData &PrimitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != PrimitiveData._pVertexBuffer)
	{
		PrimitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != PrimitiveData._pIndexBuffer)
	//{
	//	PrimitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(PrimitiveData._pMaterial.lock()->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	PrimitiveData._pMaterial.lock()->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	PrimitiveData._pMaterial.lock()->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	PrimitiveData._pMaterial.lock()->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Geometry Shader
	g_pGraphicDevice->getContext()->GSSetShader(nullptr, nullptr, 0);

	PrimitiveData._pMaterial.lock()->SetTexturesToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(PrimitiveData._pMaterial.lock()->getFillMode(), PrimitiveData._pMaterial.lock()->getCullMode()));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Enable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(PrimitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(PrimitiveData._pVertexBuffer->getVertexCount(), 0);
}

DirectionalShadowDepthPass::DirectionalShadowDepthPass()
	: RenderPass()
{
	SetUseOwningDepthStencilBuffer(ERenderTarget::DirectionalShadowDepth);
}

const bool DirectionalShadowDepthPass::processPrimitiveData(const FPrimitiveData & PrimitiveData)
{
	// 렌더 큐 만들기
	if (PrimitiveData._primitiveType != EPrimitiveType::Mesh)
	{
		return false;
	}

	return RenderPass::processPrimitiveData(PrimitiveData);
}

void DirectionalShadowDepthPass::render(const FPrimitiveData & PrimitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != PrimitiveData._pVertexBuffer)
	{
		PrimitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != PrimitiveData._pIndexBuffer)
	//{
	//	PrimitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(PrimitiveData._pMaterial.lock()->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	auto &variableInfosVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	_vertexShader->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	_vertexShader->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	auto &variableInfosPS = PrimitiveData._pMaterial.lock()->getConstantBufferVariables(ShaderType::Pixel, EConstantBufferLayer::PerObject);
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
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(PrimitiveData._pIndexBuffer-CO>getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(PrimitiveData._pVertexBuffer->getVertexCount(), 0);
}

const bool LightPass::processPrimitiveData(const FPrimitiveData &PrimitiveData)
{
	// 렌더 큐 만들기
	if (PrimitiveData._primitiveType != EPrimitiveType::Light)
	{
		return false;
	}

	PrimitiveComponent* PrimitiveComponent = PrimitiveData._pPrimitive.lock().get();

	Vec3 trans = PrimitiveComponent->getTranslation();
	Vec4 transAndRange = { trans.x, trans.y, trans.z, 10.f };
	Vec4 color = { 1.f, 1.f, 1.f, 1.f };
	if (std::shared_ptr<LightComponent> LightComp = std::static_pointer_cast<LightComponent>(PrimitiveData._pPrimitive.lock()))
	{
		color.x = LightComp->getColor().x;
		color.y = LightComp->getColor().y;
		color.z = LightComp->getColor().z;
	}
	

	XMVECTOR rotationVector = XMLoadFloat3(&PrimitiveComponent->getRotation());
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
	Mat4 rotMatrix = IDENTITYMATRIX;
	XMStoreFloat4x4(&rotMatrix, rotationMatrix);
	Vec3 look = { rotMatrix._31, rotMatrix._32, rotMatrix._33 };

	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightPosition"), transAndRange);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightDirection"), look);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_lightColor"), color);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_inverseCameraViewMatrix"), g_pMainGame->getMainCamera()->getInvesrViewMatrix());
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetValue(TEXT("g_inverseProjectiveMatrix"), g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix());

	return RenderPass::processPrimitiveData(PrimitiveData);
}

void LightPass::render(const FPrimitiveData& PrimitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != PrimitiveData._pVertexBuffer)
	{
		PrimitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != PrimitiveData._pIndexBuffer)
	//{
	//	PrimitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(PrimitiveData._pMaterial.lock()->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	//auto &variableInfosVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariableInfos(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	//PrimitiveData._pMaterial.lock()->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	PrimitiveData._pMaterial.lock()->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	//auto &variableInfosPS = PrimitiveData._pMaterial.lock()->getConstantBufferVariableInfos(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	//PrimitiveData._pMaterial.lock()->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetToDevice();

	//PrimitiveData._pMaterial.lock()->SetTexturesToDevice();

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
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(PrimitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(PrimitiveData._pVertexBuffer->getVertexCount(), 0);
}

const bool SkyPass::processPrimitiveData(const FPrimitiveData& PrimitiveData)
{
	if (PrimitiveData._primitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	return RenderPass::processPrimitiveData(PrimitiveData);
}

void SkyPass::render(const FPrimitiveData& PrimitiveData)
{
	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != PrimitiveData._pVertexBuffer)
	{
		PrimitiveData._pVertexBuffer->setBufferToDevice(stride, offset);
	}

	//if (nullptr != PrimitiveData._pIndexBuffer)
	//{
	//	PrimitiveData._pIndexBuffer->setBufferToDevice(offset);
	//}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(PrimitiveData._pMaterial.lock()->getTopology());

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	//auto &variableInfosVS = PrimitiveData._pMaterial.lock()->getConstantBufferVariableInfos(ShaderType::Vertex, EConstantBufferLayer::PerObject);
	//PrimitiveData._pMaterial.lock()->getVertexShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosVS);
	PrimitiveData._pMaterial.lock()->getVertexShader()->SetToDevice();

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	//auto &variableInfosPS = PrimitiveData._pMaterial.lock()->getConstantBufferVariableInfos(ShaderType::Pixel, EConstantBufferLayer::PerObject);
	//PrimitiveData._pMaterial.lock()->getPixelShader()->UpdateConstantBuffer(EConstantBufferLayer::PerObject, variableInfosPS);
	PrimitiveData._pMaterial.lock()->getPixelShader()->SetToDevice();

	PrimitiveData._pMaterial.lock()->SetTexturesToDevice();

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
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(PrimitiveData._pIndexBuffer->getIndexCount())
	//	, static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
	//	, static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));

	g_pGraphicDevice->getContext()->Draw(PrimitiveData._pVertexBuffer->getVertexCount(), 0);
}

PointShadowDepthPass::PointShadowDepthPass()
	: RenderPass()
{
	SetUseOwningDepthStencilBuffer(ERenderTarget::PointShadowDepth);
}
