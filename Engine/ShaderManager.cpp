#include "Include.h"
#include "ShaderManager.h"

#include "Shader.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "GeometryShader.h"

#include "ShaderLoader.h"

#include "MapUtility.h"

MShaderManager::MShaderManager()
	: _shadersPerShaderType(EnumToIndex(ShaderType::Count), ShaderMap())
{
}

MShaderManager::~MShaderManager()
{
	Release();
}

void MShaderManager::Release()
{
	for (auto &shaderList : _shadersPerShaderType)
	{
		shaderList.clear();
	}

	//_blobMapList.clear();
}

const bool MShaderManager::addShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &pShader)
{
	if (false == MapUtility::FindInsert(getShaderMap(type), fileName, pShader))
	{
		return false;
	}

	return true;
}

MShaderManager::ShaderMap &MShaderManager::getShaderMap(const ShaderType type)
{
	return _shadersPerShaderType[EnumToIndex(type)];
}

const bool MShaderManager::getShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &shader)
{
	if (false == MapUtility::FindGet(getShaderMap(type), std::wstring(fileName), shader))
	{
		DEV_ASSERT_MSG("쉐이더 파일을 찾을 수 없습니다!");
		return false;
	}

	return true;
}

const bool MShaderManager::getVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader)
{
	std::shared_ptr<MShader> pShader = nullptr;
	if (true == getShader(ShaderType::Vertex, fileName, pShader))
	{
		vertexShader = std::static_pointer_cast<VertexShader>(pShader);
		return true;
	}

	return false;
}

const bool MShaderManager::addVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader)
{
	std::shared_ptr<MShader> pShader = vertexShader;
	return addShader(ShaderType::Vertex, fileName, pShader);
}

const bool MShaderManager::getPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader)
{
	if (fileName == nullptr)
	{
		return false;
	}

	std::shared_ptr<MShader> pShader = nullptr;
	if (true == getShader(ShaderType::Pixel, fileName, pShader))
	{
		pixelShader = std::static_pointer_cast<PixelShader>(pShader);
		return true;
	}

	return false;
}

const bool MShaderManager::addPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader)
{
	std::shared_ptr<MShader> pShader = pixelShader;
	return addShader(ShaderType::Pixel, fileName, pShader);
}

const bool MShaderManager::getGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader)
{
	std::shared_ptr<MShader> pShader = nullptr;
	if (true == getShader(ShaderType::Geometry, fileName, pShader))
	{
		geometryShader = std::static_pointer_cast<MGeometryShader>(pShader);
		return true;
	}

	return false;
}

const bool MShaderManager::addGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader)
{
	std::shared_ptr<MShader> pShader = geometryShader;
	return addShader(ShaderType::Geometry, fileName, pShader);
}

MShaderManager::ShaderMap& MShaderManager::GetShaders(const ShaderType shaderType)
{
	return _shadersPerShaderType[CastValue<uint32>(shaderType)];
}

ID3D10Blob *MShaderManager::getVertexShaderBlob(const wchar_t *shaderName)
{
	std::shared_ptr<VertexShader> pShader = nullptr;
	if (true == getVertexShader(shaderName, pShader))
	{
		return pShader->getBlob();
	}

	return nullptr;
}