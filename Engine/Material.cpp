#include "stdafx.h"
#include "Material.h"

#include "MapUtility.h"

#include "MainGameSetting.h"

#include "GraphicDevice.h"
#include "TextureSetter.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

#include "ShaderManager.h"
#include "Shader.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "MainGame.h"

#include "TextureComponent.h"
#include "PrimitiveComponent.h"
#include "DynamicMeshComponent.h"

using namespace Graphic;

Material::Material()
	: _vertexShader{ nullptr }
	, _pixelShader{ nullptr }
	, _textureList(enumToIndex(TextureType::End), nullptr)

	, _variableInfosPerShaderType(CastValue<size_t>(ShaderType::Count), std::vector<std::vector<VariableInfo>>())
	, _eTopology{ D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST }
	//, _eFillMode{ FillMode::Solid }
	//, _eCullMode{ CullMode::Backface }
	//, _eDepthWriteMode{ DepthWriteMode::Enable }
	//, _eBlend{ Blend::Object }
{
}

Material::~Material()
{
	releaseShader();
}

void Material::SetTexturesToDevice()
{
	std::vector<ID3D11ShaderResourceView*> rawData;
	rawData.reserve(_textureList.size());
	for (auto &texture : _textureList)
	{
		rawData.emplace_back(texture ? texture->getRawResourceViewPointer() : nullptr);
	}

	g_pGraphicDevice->getContext()->PSSetShaderResources(0, CastValue<UINT>(rawData.size()), &rawData[0]);
}

void Material::setOwner(std::shared_ptr<PrimitiveComponent> pOwner)
{
	_pOwner = pOwner;
}

std::shared_ptr<Shader> Material::getVertexShader()
{
	return _vertexShader;
}

std::shared_ptr<Shader> Material::getPixelShader()
{
	return _pixelShader;
}

void Material::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	releaseShader();

	if (false == g_pShaderManager->getVertexShader(vertexShaderFileName, _vertexShader))
	{
		return;
	}
	_vertexShaderFileName = vertexShaderFileName;

	if (false == g_pShaderManager->getPixelShader(pixelShaderFileName, _pixelShader))
	{
		return;
	}
	_pixelShaderFileName = pixelShaderFileName;

	//-------------------------------------------------------------------------------------------------------
	size_t shaderTypeCount = CastValue<size_t>(ShaderType::Count);
	std::vector<std::shared_ptr<Shader>> shaders(shaderTypeCount, nullptr);
	shaders[CastValue<uint32>(ShaderType::Vertex)]	= _vertexShader;
	shaders[CastValue<uint32>(ShaderType::Pixel)]	= _pixelShader;

	for (size_t shaderTypeIndex = 0; shaderTypeIndex < shaderTypeCount; ++shaderTypeIndex)
	{
		_variableInfosPerShaderType[shaderTypeIndex] = shaders[shaderTypeIndex]->getVariableInfos(); // 각 쉐이더의 콘스탄트 버퍼 변수 정보를 등록
	}
}

void Material::releaseShader()
{
	_vertexShaderFileName.clear();
	_pixelShaderFileName.clear();
}

void Material::setTexture(const TextureType textureType, std::shared_ptr<TextureComponent> pTexture)
{
	_textureList[enumToIndex(textureType)] = pTexture;
}

void Material::setTexture(std::vector<std::shared_ptr<TextureComponent>> &textureList)
{
	_textureList = textureList;
}

void Material::setTopology(const D3D_PRIMITIVE_TOPOLOGY eTopology)
{
	_eTopology = eTopology;
}

const D3D_PRIMITIVE_TOPOLOGY Material::getTopology() const
{
	return _eTopology;
}

std::vector<VariableInfo>& Material::getConstantBufferVariableInfos(const ShaderType shaderType, const uint32 index)
{
	return _variableInfosPerShaderType[CastValue<uint32>(shaderType)][index];
}

std::vector<VariableInfo>& Material::getConstantBufferVariableInfos(const ShaderType shaderType, const ConstantBuffersLayer layer)
{
	return _variableInfosPerShaderType[CastValue<uint32>(shaderType)][CastValue<uint32>(layer)];
}

const bool Material::useTextureType(const TextureType type)
{
	return _textureList[CastValue<uint32>(type)] != nullptr ? true : false;
}

//void Material::setFillMode(const Graphic::FillMode eFillMode)
//{
//	_eFillMode = eFillMode;
//}
//
//void Material::setCullMode(const Graphic::CullMode eCullMode)
//{
//	_eCullMode = eCullMode;
//}
//
//void Material::setDepthWriteMode(const Graphic::DepthWriteMode eDepthWriteMode)
//{
//	_eDepthWriteMode = eDepthWriteMode;
//}
//
//void Material::setBlendState(const Graphic::Blend eBlend)
//{
//	_eBlend = eBlend;
//}
