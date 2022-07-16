#include "stdafx.h"
#include "ShaderManager.h"

#include "Shader.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "ShaderLoader.h"

#include "MapUtility.h"

ShaderManager::ShaderManager()
	: _shadersPerShaderType(enumToIndex(ShaderType::Count), ShaderMap())
{
}

ShaderManager::~ShaderManager()
{
	Release();
}

void ShaderManager::Release()
{
	for (auto &shaderList : _shadersPerShaderType)
	{
		shaderList.clear();
	}

	//_blobMapList.clear();
}

const bool ShaderManager::addShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<Shader> &pShader)
{
	if (false == MapUtility::FindInsert(getShaderMap(type), fileName, pShader))
	{
		return false;
	}

	return true;
}

ShaderManager::ShaderMap &ShaderManager::getShaderMap(const ShaderType type)
{
	return _shadersPerShaderType[enumToIndex(type)];
}

const bool ShaderManager::getShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<Shader> &shader)
{
	if (false == MapUtility::FindGet(getShaderMap(type), std::wstring(fileName), shader))
	{
		DEV_ASSERT_MSG("쉐이더 파일을 찾을 수 없습니다!");
		return false;
	}

	return true;
}

const bool ShaderManager::getVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader)
{
	std::shared_ptr<Shader> pShader = nullptr;
	if (true == getShader(ShaderType::Vertex, fileName, pShader))
	{
		vertexShader = std::static_pointer_cast<VertexShader>(pShader);
		return true;
	}

	return false;
}

const bool ShaderManager::addVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader)
{
	std::shared_ptr<Shader> pShader = vertexShader;
	return addShader(ShaderType::Vertex, fileName, pShader);
}

const bool ShaderManager::getPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader)
{
	std::shared_ptr<Shader> pShader = nullptr;
	if (true == getShader(ShaderType::Pixel, fileName, pShader))
	{
		pixelShader = std::static_pointer_cast<PixelShader>(pShader);
		return true;
	}

	return false;
}

const bool ShaderManager::addPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader)
{
	std::shared_ptr<Shader> pShader = pixelShader;
	return addShader(ShaderType::Pixel, fileName, pShader);
}

ShaderManager::ShaderMap& ShaderManager::getShaders(const ShaderType shaderType)
{
	return _shadersPerShaderType[CastValue<uint32>(shaderType)];
}

ID3D10Blob *ShaderManager::getVertexShaderBlob(const wchar_t *shaderName)
{
	std::shared_ptr<VertexShader> pShader = nullptr;
	if (true == getVertexShader(shaderName, pShader))
	{
		return pShader->getBlob();
	}

	return nullptr;
}