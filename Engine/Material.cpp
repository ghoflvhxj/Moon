#include "Material.h"

#include "MapUtility.h"

#include "MainGameSetting.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

#include "ShaderManager.h"
#include "Shader.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "Core/ResourceManager.h"
#include "Core/Serialize/JsonDeSerializer.h"

#include "PrimitiveComponent.h"
#include "DynamicMeshComponent.h"

using namespace Graphic;

MMaterial::MMaterial()
	: _textureList(EnumToIndex(ETextureType::End), nullptr)
	, _eTopology{ D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST }
	, _eFillMode{ FillMode::Solid }
	, _eCullMode{ CullMode::Backface }
	, bUseAlpha{ false }
	, bAlphaMask{ false }
{
    setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso"));
}

MMaterial::~MMaterial()
{
	ClearShader();
    //LOG(std::wstring(TEXT("Destroy MMaterial!!!")));
}

void MMaterial::OnLoaded()
{
    Super::OnLoaded();
}

void MMaterial::SetTexturesToDevice()
{
	std::vector<ID3D11ShaderResourceView*> rawData;
	rawData.reserve(_textureList.size());
	for (auto &texture : _textureList)
	{
        if (texture.get() == nullptr)
        {
            rawData.push_back(nullptr);
        }
        else
        {
            rawData.push_back(texture->getRawResourceViewPointer());
        }

		//rawData.push_back(texture != nullptr ? texture->getRawResourceViewPointer() : nullptr);
	}

	g_pGraphicDevice->getContext()->PSSetShaderResources(0, CastValue<UINT>(rawData.size()), &rawData[0]);

	ID3D11SamplerState* SamplerState = IsAlphaMasked() ? g_pGraphicDevice->getSamplerState(ESamplerFilter::Point) : g_pGraphicDevice->getSamplerState(ESamplerFilter::Anisotropic);
	if (SamplerState)
	{
		g_pGraphicDevice->getContext()->PSSetSamplers(0, 1, &SamplerState);
	}
}

std::shared_ptr<MShader> MMaterial::getVertexShader()
{
    std::shared_ptr<VertexShader>	_vertexShader;
    g_pGraphicDevice->GetVertexShader(_vertexShaderFileName.c_str(), _vertexShader);
    return _vertexShader;
}

std::shared_ptr<MShader> MMaterial::getPixelShader()
{
    std::shared_ptr<PixelShader>	_pixelShader;
    g_pGraphicDevice->GetPixelShader(_pixelShaderFileName.c_str(), _pixelShader);
    return _pixelShader;
}

void MMaterial::setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName)
{
	ClearShader();

	//if (false == ShaderManager->getVertexShader(vertexShaderFileName, _vertexShader))
	//{
	//	return;
	//}
	_vertexShaderFileName = vertexShaderFileName;

	//if (false == ShaderManager->getPixelShader(pixelShaderFileName, _pixelShader))
	//{
	//	return;
	//}
	_pixelShaderFileName = pixelShaderFileName;
}

void MMaterial::ClearShader()
{
	_vertexShaderFileName.clear();
	_pixelShaderFileName.clear();
}

void MMaterial::setTexture(const ETextureType textureType, std::shared_ptr<MTexture> pTexture)
{
	_textureList[EnumToIndex(textureType)] = pTexture;
}

void MMaterial::setTextures(std::vector<std::shared_ptr<MTexture>> &textureList)
{
	_textureList = textureList;
}

void MMaterial::setTopology(const D3D_PRIMITIVE_TOPOLOGY eTopology)
{
	_eTopology = eTopology;
}

void MMaterial::setFillMode(const Graphic::FillMode fillMode)
{
	_eFillMode = fillMode;
}

void MMaterial::setCullMode(const Graphic::CullMode cullMode)
{
	_eCullMode = cullMode;
}

const D3D_PRIMITIVE_TOPOLOGY MMaterial::getTopology() const
{
	return _eTopology;
}

const Graphic::FillMode MMaterial::getFillMode() const
{
	return _eFillMode;
}

const Graphic::CullMode MMaterial::getCullMode() const
{
	return _eCullMode;
}

std::vector<FBufferVariable>& MMaterial::getConstantBufferVariables(const ShaderType InShaderType, const uint32 index)
{
	return getConstantBufferVariables(InShaderType, static_cast<EConstantBufferLayer>(index));
}

std::vector<FBufferVariable>& MMaterial::getConstantBufferVariables(const ShaderType InShaderType, const EConstantBufferLayer layer)
{
	//size_t shaderTypeCount = CastValue<size_t>(ShaderType::Count);
	//std::vector<std::shared_ptr<MShader>> shaders(shaderTypeCount, nullptr);
	//shaders[CastValue<uint32>(ShaderType::Vertex)] = _vertexShader;
	//shaders[CastValue<uint32>(ShaderType::Pixel)] = _pixelShader;

	//uint32 shaderTypeIndex = static_cast<uint32>(shaderType);
	//if (shaders[shaderTypeIndex] == nullptr)
	//{
	//	static std::vector<FShaderVariable> Dummy;
	//	return Dummy;
	//}

	//return shaders[shaderTypeIndex]->GetVariables()[CastValue<uint32>(layer)];

    std::shared_ptr<MShader> Shader = nullptr;

    switch (InShaderType)
    {
    case ShaderType::Vertex:
    {
        Shader = getVertexShader();
        break;
    }
    case ShaderType::Pixel:
    {
        Shader = getPixelShader();
        break;
    }
    }

    if (Shader == nullptr)
    {
        static std::vector<FBufferVariable> Dummy;
        return Dummy;
    }

    return Shader->GetVariables()[CastValue<uint32>(layer)];
}

const bool MMaterial::IsTextureTypeUsed(const ETextureType type)
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
