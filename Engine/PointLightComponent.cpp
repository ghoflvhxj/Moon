#include "stdafx.h"
#include "PointLightComponent.h"

#include "Render.h"

#include "Material.h"

#include "StaticMeshComponent.h"

using namespace DirectX;

PointLightComponent::PointLightComponent(void)
	: LightComponent()
	, _range{ 1.f }
{
	getMesh()->getMaterial(0)->setShader(TEXT("Light.cso"), TEXT("PointLightShader.cso"));
}

PointLightComponent::~PointLightComponent(void)
{
}

void PointLightComponent::Update(const Time deltaTime)
{
	PrimitiveComponent::Update(deltaTime);

	XMVECTOR scaleVector = XMLoadFloat3(&getScale());
	XMVECTOR trasnlationVector = XMLoadFloat3(&getTranslation());
	XMMATRIX IdentityMatrix = XMLoadFloat4x4(&IDENTITYMATRIX);
	
	XMMATRIX matrices[(int)Transform::End] = {
		XMMatrixScalingFromVector(scaleVector),
		IdentityMatrix,
		XMMatrixTranslationFromVector(trasnlationVector)
	};

	XMStoreFloat4x4(&getWorldMatrix(), matrices[(int)Transform::Scale] * matrices[(int)Transform::Rotation] * matrices[(int)Transform::Translation]);
}

const bool PointLightComponent::getPrimitiveData(PrimitiveData &primitiveData)
{
	LightComponent::getPrimitiveData(primitiveData);

	primitiveData._pMaterial = getMesh()->getMaterial(0);
	primitiveData._pVertexShader = getMesh()->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = getMesh()->getMaterial(0)->getPixelShader();

	return true;
}

//void PointLightComponent::render()
//{
	//VertexShader vertexShader = nullptr;
	//if (false == g_pShaderManager->getVertexShader(TEXT("TexVertexShader.cso"), vertexShader))
	//{
	//	return;
	//}

	//PixelShader pixelShader = nullptr;
	//if (false == g_pShaderManager->getPixelShader(TEXT("PointLightShader.cso"), pixelShader))
	//{
	//	return;
	//}

	//g_pGraphicDevice->SetVertexShader(vertexShader);
	//g_pGraphicDevice->SetPixelShader(pixelShader);

	////---------------------------------------------------------------------------------------------------------------------------------
	//// Input Assembler
	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;
	//_pVertexBuffer->setBufferToDevice(stride, offset);
	//_pIndexBuffer->setBufferToDevice(offset);

	//g_pGraphicDevice->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	////---------------------------------------------------------------------------------------------------------------------------------
	//// Vertex Shader
	//{
	//	VertexShaderConstantBuffer data = {
	//		IDENTITYMATRIX,
	//		IDENTITYMATRIX,
	//		g_pMainGame->getMainCamera()->getOrthographicProjectionMatrix()
	//	};

	//	auto pConstantBuffer = _pVertexConstantBuffer->getRaw();
	//	_pVertexConstantBuffer->update(&data, sizeof(VertexShaderConstantBuffer));

	//	g_pGraphicDevice->getContext()->VSSetConstantBuffers(0u, 1u, &pConstantBuffer);
	//}

	////g_pGraphicDevice->getContext()->VSSetShader(nullptr, nullptr, 0);

	////---------------------------------------------------------------------------------------------------------------------------------
	//// Pixel Shader
	//{
	//	Mat4 invView = g_pMainGame->getMainCamera()->getInvesrViewMatrix();
	//	XMMATRIX invViewMat = XMLoadFloat4x4(&invView);
	//	invViewMat = XMMatrixTranspose(invViewMat);
	//	XMStoreFloat4x4(&invView, invViewMat);

	//	Mat4 invProj = g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix();
	//	XMMATRIX invProjMat = XMLoadFloat4x4(&invProj);
	//	invProjMat = XMMatrixTranspose(invProjMat);
	//	XMStoreFloat4x4(&invProj, invProjMat);

	//	const Vec3 color = getColor();
	//	Vec3 translation = getWorldTranslation();
	//	PixelShaderConstantBuffer data = {
	//		Vec4{translation.x, translation.y, translation.z, _range},
	//		Vec4{color.x, color.y, color.z, getIntensity()},
	//		invView,
	//		invProj,
	//	};

	//	auto pPSConstantBuffer = _pPixelConstantBuffer->getRaw();
	//	_pPixelConstantBuffer->update(&data, sizeof(PixelShaderConstantBuffer));
	//	g_pGraphicDevice->getContext()->PSSetConstantBuffers(0u, 1u, &pPSConstantBuffer);
	//}
	////---------------------------------------------------------------------------------------------------------------------------------
	//// RasterizerState
	//g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	////--------------------------------------------------------------------------------------------------------------------------------
	//// DepthStencilState
	//g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Enable), 1);

	////--------------------------------------------------------------------------------------------------------------------------------
	//// OutputMerge
	//g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);

	////---------------------------------------------------------------------------------------------------------------------------------
	//g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(_indexList.size()), 0u, 0u);
//}

void PointLightComponent::addRange(float addRange)
{
	_range += addRange;
}

void PointLightComponent::setRange(float range)
{
	_range = range;
}

const float PointLightComponent::getRange() const
{
	return _range;
}
