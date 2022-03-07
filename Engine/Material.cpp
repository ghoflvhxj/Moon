#include "stdafx.h"
#include "Material.h"

#include "MainGameSetting.h"

#include "GraphicDevice.h"
#include "TextureSetter.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

#include "ShaderManager.h"

#include "MainGame.h"

#include "TextureComponent.h"
#include "PrimitiveComponent.h"

using namespace Graphic;

Material::Material(std::vector<Vertex> &vertexList)
	: _vertexOffsetList(1, 0)
	, _indexOffsetList(1, 0)
	, _pVertexBuffer{ nullptr }
	, _pIndexBuffer{ nullptr }
	, _pVertexConstantBuffer{ nullptr }
	, _pPixelConstantBuffer{ nullptr }
	, _pVertexShader{ nullptr }
	, _pPixelShader{ nullptr }
	, _textureList(enumToUInt32(TextureType::End), nullptr)

	, _eTopology{ D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST }
	, _eFillMode{ FillMode::Solid }
	, _eCullMode{ CullMode::Backface }
	, _eDepthWriteMode{ DepthWriteMode::Enable }
	, _eBlend{ Blend::Object }
{
	_vertexOffsetList.push_back(static_cast<uint32>(vertexList.size()));

	initializeBuffers(vertexList);
	initializeConstantBuffers();
}

Material::Material(std::vector<Vertex> &vertexList, std::vector<Index> &indexList)
	: _vertexOffsetList(1, 0)
	, _indexOffsetList(1, 0)
	, _pVertexBuffer		{ nullptr }
	, _pIndexBuffer			{ nullptr }
	, _pVertexConstantBuffer{ nullptr }
	, _pPixelConstantBuffer	{ nullptr }
	, _pVertexShader		{ nullptr }
	, _pPixelShader			{ nullptr }
	, _textureList(enumToUInt32(TextureType::End), nullptr)

	, _eTopology{ D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST }
	, _eFillMode{ FillMode::Solid }
	, _eCullMode{ CullMode::Backface }
	, _eDepthWriteMode{ DepthWriteMode::Enable }
	, _eBlend{ Blend::Object }

{
	_vertexOffsetList.push_back(static_cast<uint32>(vertexList.size()));
	_indexOffsetList.push_back(static_cast<uint32>(indexList.size()));

	initializeBuffers(vertexList, indexList);
	initializeConstantBuffers();
}

Material::Material(std::vector<VertexList> &verticesList)
	: _vertexOffsetList(1, 0)
	, _indexOffsetList(1, 0)
	, _pVertexBuffer{ nullptr }
	, _pIndexBuffer{ nullptr }
	, _pVertexConstantBuffer{ nullptr }
	, _pPixelConstantBuffer{ nullptr }
	, _pVertexShader{ nullptr }
	, _pPixelShader{ nullptr }
	, _textureList(enumToUInt32(TextureType::End), nullptr)

	, _eTopology{ D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST }
	, _eFillMode{ FillMode::Solid }
	, _eCullMode{ CullMode::Backface }
	, _eDepthWriteMode{ DepthWriteMode::Enable }
	, _eBlend{ Blend::Object }
{
	VertexList vertexList;



	initializeBuffers(vertexList);
	initializeConstantBuffers();
}

Material::Material(std::vector<VertexList> &verticesList, std::vector<IndexList> &indicesList)
	: _vertexOffsetList(1, 0)
	, _indexOffsetList(1, 0)
	, _pVertexBuffer{ nullptr }
	, _pIndexBuffer{ nullptr }
	, _pVertexConstantBuffer{ nullptr }
	, _pPixelConstantBuffer{ nullptr }
	, _pVertexShader{ nullptr }
	, _pPixelShader{ nullptr }
	, _textureList(enumToUInt32(TextureType::End), nullptr)

	, _eTopology{ D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST }
	, _eFillMode{ FillMode::Solid }
	, _eCullMode{ CullMode::Backface }
	, _eDepthWriteMode{ DepthWriteMode::Enable }
	, _eBlend{ Blend::Object }
{
	VertexList vertexList;

	uint32 vertexCount = 0;
	uint32 listCount = static_cast<uint32>(verticesList.size());
	for (uint32 index = 0; index < listCount; ++index)
	{
		vertexCount += static_cast<uint32>(verticesList[index].size());
	}

	_vertexOffsetList.reserve(listCount);
	vertexList.reserve(vertexCount);
	for (uint32 index = 0; index < listCount; ++index)
	{
		vertexList.insert(vertexList.end(), verticesList[index].begin(), verticesList[index].end());
		_vertexOffsetList.push_back(static_cast<uint32>(vertexList.size()));
	}

	initializeBuffers(vertexList);
	initializeConstantBuffers();
}

Material::~Material()
{
}

void Material::initializeVertexListFromVerticesList(VertexList &vertexList, std::vector<VertexList> &verticesList)
{
	uint32 vertexCount = 0;
	uint32 listCount = static_cast<uint32>(verticesList.size());
	for (uint32 index = 0; index < listCount; ++index)
	{
		vertexCount += static_cast<uint32>(verticesList[index].size());
	}

	_vertexOffsetList.reserve(listCount);
	vertexList.reserve(vertexCount);
	for (uint32 index = 0; index < listCount; ++index)
	{
		vertexList.insert(vertexList.end(), verticesList[index].begin(), verticesList[index].end());
		_vertexOffsetList.push_back(static_cast<uint32>(vertexList.size()));
	}
}

void Material::initializeIndexListFromIndicesList(IndexList &indexList, std::vector<IndexList> &IndicesList)
{
	uint32 indexCount = 0;
	uint32 listCount = static_cast<uint32>(IndicesList.size());
	for (uint32 index = 0; index < listCount; ++index)
	{
		indexCount += static_cast<uint32>(IndicesList[index].size());
	}

	_indexOffsetList.reserve(listCount);
	IndicesList.reserve(indexCount);
	for (uint32 index = 0; index < listCount; ++index)
	{
		indexList.insert(indexList.end(), IndicesList[index].begin(), IndicesList[index].end());
		_vertexOffsetList.push_back(static_cast<uint32>(indexList.size()));
	}
}

void Material::initializeBuffers(std::vector<Vertex> &vertexList)
{
	_pVertexBuffer = std::make_shared<VertexBuffer>(static_cast<int>(sizeof(Vertex)), static_cast<int>(vertexList.size()), &vertexList[0]);
}

void Material::initializeBuffers(std::vector<Vertex> &vertexList, std::vector<Index> &indexList)
{
	_pVertexBuffer	= std::make_shared<VertexBuffer>(static_cast<int>(sizeof(Vertex)), static_cast<int>(vertexList.size()), &vertexList[0]);
	_pIndexBuffer	= std::make_shared<IndexBuffer>(static_cast<int>(sizeof(Index)), static_cast<int>(indexList.size()), &indexList[0]);
}

void Material::initializeConstantBuffers()
{
	// VertexShader Constant Buffer
	{
		VertexShaderConstantBuffer data = {
			IDENTITYMATRIX,
			IDENTITYMATRIX,
			IDENTITYMATRIX
		};

		_pVertexConstantBuffer = std::make_shared<ConstantBuffer>(static_cast<int>(sizeof(VertexShaderConstantBuffer)), &data);
	}

	// PixelShader Constant Buffer
	{
		PixelShaderConstantBuffer data = {
			false,
			false
		};

		_pPixelConstantBuffer = std::make_shared<ConstantBuffer>(static_cast<int>(sizeof(PixelShaderConstantBuffer)), &data);
	}
}

void Material::setOwner(std::shared_ptr<PrimitiveComponent> pOwner)
{
	_pOwner = pOwner;
}

void Material::setShader(const wchar_t *vertexShader, const wchar_t *pixelShader)
{
	if (false == g_pShaderManager->getVertexShader(vertexShader, &_pVertexShader))
	{
		DEV_ASSERT_MSG(TEXT("쉐이더 파일을 찾을 수 없습니다!"));
	}

	if (false == g_pShaderManager->getPixelShader(pixelShader, &_pPixelShader))
	{
		DEV_ASSERT_MSG(TEXT("쉐이더 파일을 찾을 수 없습니다!"));
	}
}

void Material::render(std::shared_ptr<PrimitiveComponent> pComponent)
{
	ASSERT_CONDITION_MSG(nullptr != pComponent, TEXT("Material은 렌더링 시 PrimitiveComponent 타입의 Owner가 필요합니다!"));
	ASSERT_CONDITION_MSG(nullptr != _pVertexShader, TEXT("Material에 버텍스 셰이더가 설정되지 않았습니다!"));
	ASSERT_CONDITION_MSG(nullptr != _pPixelShader, TEXT("Material에 픽셀 셰이더가 설정되지 않았습니다!"));

	//---------------------------------------------------------------------------------------------------------------------------------
	// Input Assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	if (nullptr != _pVertexBuffer)
	{
		_pVertexBuffer->setBufferToDevice(stride, offset);
	}

	if (nullptr != _pIndexBuffer)
	{
		_pIndexBuffer->setBufferToDevice(offset);
	}

	g_pGraphicDevice->getContext()->IASetPrimitiveTopology(_eTopology);

	//---------------------------------------------------------------------------------------------------------------------------------
	// Vertex Shader
	g_pGraphicDevice->getContext()->VSSetShader(_pVertexShader, nullptr, 0);
	{
		VertexShaderConstantBuffer data = {
			pComponent->getWorldMatrix(),
			g_pMainGame->getMainCameraViewMatrix(),
			g_pMainGame->getMainCameraProjectioinMatrix()
		};

		if (PrimitiveComponent::RenderMode::Orthogonal == pComponent->getRenderMdoe())
		{
			data.cameraViewMatrix = IDENTITYMATRIX;
			data.projectionMatrix = g_pMainGame->getMainCameraOrthographicProjectionMatrix();
		}

		auto pConstantBuffer = _pVertexConstantBuffer->getBuffer();
		_pVertexConstantBuffer->Update(&data, sizeof(VertexShaderConstantBuffer));

		g_pGraphicDevice->getContext()->VSSetConstantBuffers(0u, 1u, &pConstantBuffer);
	}

	//---------------------------------------------------------------------------------------------------------------------------------
	// Pixel Shader
	g_pGraphicDevice->getContext()->PSSetShader(_pPixelShader, nullptr, 0);
	{
		PixelShaderConstantBuffer data = {
			false,
			false,
		};

		if (nullptr != _textureList[enumToIndex(TextureType::Normal)])
		{
			data.usingNormalTexture = true;
		}

		if (nullptr != _textureList[enumToIndex(TextureType::Specular)])
		{
			data.usingSepcuarTexture = true;
		}

		auto pPSConstantBuffer = _pPixelConstantBuffer->getBuffer();
		_pPixelConstantBuffer->Update(&data, sizeof(PixelShaderConstantBuffer));
		g_pGraphicDevice->getContext()->PSSetConstantBuffers(0u, 1u, &pPSConstantBuffer);
	}

	TextureSetter textureSetter(_textureList);	// 자동으로 ShaderResourceView를 세팅하고 해제해준다

	//---------------------------------------------------------------------------------------------------------------------------------
	// RasterizerState
	g_pGraphicDevice->getContext()->RSSetState(g_pGraphicDevice->getRasterizerState(_eFillMode, _eCullMode));

	//--------------------------------------------------------------------------------------------------------------------------------
	// DepthStencilState
	g_pGraphicDevice->getContext()->OMSetDepthStencilState(g_pGraphicDevice->getDepthStencilState(_eDepthWriteMode), 1);

	//--------------------------------------------------------------------------------------------------------------------------------
	// OutputMerge
	g_pGraphicDevice->getContext()->OMSetBlendState(g_pGraphicDevice->getBlendState(_eBlend), nullptr, 0xffffffff);

	//--------------------------------------------------------------------------------------------------------------------------------
	if (nullptr != _pIndexBuffer)
	{
		uint32 indexOffsetCount = static_cast<uint32>(_indexOffsetList.size());
		for (uint32 index = 0; index + 1 < indexOffsetCount; ++index)
		{
			g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(_indexOffsetList[index + 1] - _indexOffsetList[index])
													  , static_cast<UINT>(_indexOffsetList[index])
													  , static_cast<UINT>(_vertexOffsetList[index]));
		}
		g_pGraphicDevice->getContext()->DrawIndexed(static_cast<UINT>(_pIndexBuffer->getIndexCount() - _indexOffsetList[indexOffsetCount - 1])
													  , static_cast<UINT>(_indexOffsetList[indexOffsetCount - 1])
													  , static_cast<UINT>(_vertexOffsetList[indexOffsetCount - 1]));
	}
	else
	{
		uint32 vertexOffsetCount = static_cast<uint32>(_vertexOffsetList.size());
		for (uint32 index = 0; index + 1 < vertexOffsetCount; ++index)
		{
			g_pGraphicDevice->getContext()->Draw(static_cast<UINT>(_vertexOffsetList[index + 1] - _vertexOffsetList[index])
											   , static_cast<UINT>(_vertexOffsetList[index]));
		}
		g_pGraphicDevice->getContext()->Draw(static_cast<UINT>(_pVertexBuffer->getVertexCount() - _vertexOffsetList[vertexOffsetCount - 1])
										   , static_cast<UINT>(_vertexOffsetList[vertexOffsetCount - 1]));
	}
}

void Material::setTexture(const TextureType textureType, std::shared_ptr<TextureComponent> pTexture)
{
	_textureList[enumToUInt32(textureType)] = pTexture;
}

void Material::setTexture(std::vector<std::shared_ptr<TextureComponent>> &textureList)
{
	_textureList = textureList;
}

void Material::setTopology(const D3D_PRIMITIVE_TOPOLOGY eTopology)
{
	_eTopology = eTopology;
}

void Material::setFillMode(const Graphic::FillMode eFillMode)
{
	_eFillMode = eFillMode;
}

void Material::setCullMode(const Graphic::CullMode eCullMode)
{
	_eCullMode = eCullMode;
}

void Material::setDepthWriteMode(const Graphic::DepthWriteMode eDepthWriteMode)
{
	_eDepthWriteMode = eDepthWriteMode;
}

void Material::setBlendState(const Graphic::Blend eBlend)
{
	_eBlend = eBlend;
}
