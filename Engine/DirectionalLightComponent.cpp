#include "stdafx.h"
#include "DirectionalLightComponent.h"

#include "MainGameSetting.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

#include "ShaderManager.h"

#include "MainGame.h"

#include "Camera.h"

using namespace DirectX;

DirectionalLightComponent::DirectionalLightComponent(void)
	: LightComponent()
{
	initializeVertices();
	initializeBuffers();
}

DirectionalLightComponent::~DirectionalLightComponent(void)
{

}


void DirectionalLightComponent::initializeVertices()
{
	float halfWidth = g_pSetting->getResolutionWidth() / 2.f;
	float halfHeight = g_pSetting->getResolutionHeight() / 2.f;

	_vertexList.push_back({ Vec3{ -halfWidth, halfHeight, 1.f }, static_cast<Vec4>(Colors::White), Vec2{0.f, 0.f}, Vec3{0.f, 0.f, -1.f} });
	_vertexList.push_back({ Vec3{ halfWidth, halfHeight, 1.f }, static_cast<Vec4>(Colors::Red), Vec2(1.f, 0.f), Vec3{0.f, 0.f, -1.f} });
	_vertexList.push_back({ Vec3{ halfWidth, -halfHeight, 1.f }, static_cast<Vec4>(Colors::Green), Vec2(1.f, 1.f), Vec3{0.f, 0.f, -1.f} });
	_vertexList.push_back({ Vec3{ -halfWidth, -halfHeight, 1.f }, static_cast<Vec4>(Colors::Yellow), Vec2(0.f, 1.f), Vec3{0.f, 0.f, -1.f} });

	_indexList.push_back(0);
	_indexList.push_back(1);
	_indexList.push_back(2);

	_indexList.push_back(0);
	_indexList.push_back(2);
	_indexList.push_back(3);

	_pVertexBuffer = std::make_shared<VertexBuffer>(static_cast<uint32>(sizeof(Vertex)), 4, &_vertexList[0]);
	_pIndexBuffer = std::make_shared<IndexBuffer>(static_cast<uint32>(sizeof(Index)), 6, &_indexList[0]);
}

void DirectionalLightComponent::initializeBuffers()
{
	_pVertexBuffer = std::make_shared<VertexBuffer>(static_cast<int>(sizeof(Vertex)), static_cast<int>(_vertexList.size()), &_vertexList[0]);

	{
		VertexShaderConstantBuffer data = {
			IDENTITYMATRIX,
			IDENTITYMATRIX,
			IDENTITYMATRIX
		};

		_pVertexConstantBuffer = std::make_shared<ConstantBuffer>(static_cast<int>(sizeof(VertexShaderConstantBuffer)), &data);
	}

	{
		PixelShaderConstantBuffer data = {
			Vec4{0.f, 0.f, 0.f, 0.f},
			Vec4{0.f, 0.f, 0.f, getIntensity()},
			IDENTITYMATRIX,
			IDENTITYMATRIX
		};

		_pPixelConstantBuffer = std::make_shared<ConstantBuffer>(static_cast<int>(sizeof(PixelShaderConstantBuffer)), &data);
	}
}

void DirectionalLightComponent::render()
{
	ID3D11VertexShader *pVertexShader = nullptr;
	if (false == g_pShaderManager->getVertexShader(TEXT("TexVertexShader.cso"), &pVertexShader))
	{
		return;
	}

	ID3D11PixelShader *pPixelShader = nullptr;
	if (false == g_pShaderManager->getPixelShader(TEXT("DirectionalLightShader.cso"), &pPixelShader))
	{
		return;
	}

	g_pGraphicDevice->getContext()->VSSetShader(pVertexShader, nullptr, 0);
	g_pGraphicDevice->getContext()->PSSetShader(pPixelShader, nullptr, 0);

	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	_pVertexBuffer->setBufferToDevice(stride, offset);
	_pIndexBuffer->setBufferToDevice(offset);

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	{
		VertexShaderConstantBuffer data = {
			IDENTITYMATRIX,
			IDENTITYMATRIX,
			g_pMainGame->getMainCamera()->getOrthographicProjectionMatrix()
		};

		auto pConstantBuffer = _pVertexConstantBuffer->getBuffer();
		_pVertexConstantBuffer->Update(&data, sizeof(VertexShaderConstantBuffer));

		g_pGraphicDevice->getContext()->VSSetConstantBuffers(0u, 1u, &pConstantBuffer);
	}

	//g_pGraphicDevice->getContext()->VSSetShader(nullptr, nullptr, 0);

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	{
		Mat4 invView = g_pMainGame->getMainCamera()->getInvesrViewMatrix();
		XMMATRIX invViewMat = XMLoadFloat4x4(&invView);
		invViewMat = XMMatrixTranspose(invViewMat);
		XMStoreFloat4x4(&invView, invViewMat);

		Mat4 invProj = g_pMainGame->getMainCamera()->getInversePerspectiveProjectionMatrix();
		XMMATRIX invProjMat = XMLoadFloat4x4(&invProj);
		invProjMat = XMMatrixTranspose(invProjMat);
		XMStoreFloat4x4(&invProj, invProjMat);

		const Vec3 color = getColor();
		Vec3 direction = getUp();
		PixelShaderConstantBuffer data = {
			Vec4{direction.x, direction.y, direction.z, 0.f},
			Vec4{color.x, color.y, color.z, getIntensity()},
			invView,
			invProj,
		};

		auto pPSConstantBuffer = _pPixelConstantBuffer->getBuffer();
		_pPixelConstantBuffer->Update(&data, sizeof(PixelShaderConstantBuffer));
		g_pGraphicDevice->getContext()->PSSetConstantBuffers(0u, 1u, &pPSConstantBuffer);
	}
	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(Graphic::FillMode::Solid, Graphic::CullMode::Backface));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(Graphic::DepthWriteMode::Enable), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(Graphic::Blend::Light), nullptr, 0xffffffff);

	//---------------------------------------------------------------------------------------------------------------------------------
	g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(_indexList.size()), 0u, 0u);
}
