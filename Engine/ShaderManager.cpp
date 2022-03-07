#include "stdafx.h"
#include "ShaderManager.h"

#include "MapUtility.h"

#include "ShaderLoader.h"

ShaderManager::ShaderManager()
	: _shaderMapList(EnumIndex(ShaderType::End), ShaderMap())
	, _blobMapList(EnumIndex(ShaderType::End), BlobMap())
{
}

ShaderManager::~ShaderManager()
{
	releaseShader();
	releaseBlob();
}

void ShaderManager::releaseShader()
{
	for (auto &shaderMap : _shaderMapList)
	{
		for (auto iter = shaderMap.begin(); iter != shaderMap.end(); ++iter)
		{
			iter->second->Release();
		}
	}

	_blobMapList.clear();
}

ShaderManager::ShaderMap &ShaderManager::getShaderMap(const ShaderType type)
{
	return _shaderMapList[EnumIndex(type)];
}

const bool ShaderManager::getShader(const ShaderType type, const wchar_t *fileName, ID3D11DeviceChild **pShader)
{
	return MapUtility::FindGet(getShaderMap(type), std::wstring(fileName), pShader);
}

const bool ShaderManager::getVertexShader(const wchar_t *fileName, ID3D11VertexShader **pShader)
{
	return getShader(ShaderType::Vertex, fileName, reinterpret_cast<ID3D11DeviceChild**>(pShader));
}

const bool ShaderManager::addVertexShader(const wchar_t *fileName, ID3D11VertexShader *pShader, ID3D10Blob *pBlob)
{
	if (false == MapUtility::FindInsert(getShaderMap(ShaderType::Vertex), fileName, pShader))
	{
		return false;
	}

	if (false == MapUtility::FindInsert(getBlobMap(ShaderType::Vertex), fileName, pBlob))
	{
		return false;
	}

	return true;
}

const bool ShaderManager::getPixelShader(const wchar_t *fileName, ID3D11PixelShader **pShader)
{
	return getShader(ShaderType::Pixel, fileName, reinterpret_cast<ID3D11DeviceChild**>(pShader));
}

const bool ShaderManager::addPixelShader(const wchar_t *fileName, ID3D11PixelShader *pShader, ID3D10Blob *pBlob)
{
	if (false == MapUtility::FindInsert(getShaderMap(ShaderType::Pixel), fileName, pShader))
	{
		return false;
	}

	if (false == MapUtility::FindInsert(getBlobMap(ShaderType::Pixel), fileName, pBlob))
	{
		return false;
	}

	return true;
}

ID3D10Blob *ShaderManager::getVertexShaderBlob(const wchar_t *shaderName)
{
	return MapUtility::FindGet(getBlobMap(ShaderType::Vertex), shaderName);
}

void ShaderManager::releaseBlob()
{
	for (auto &blobMap : _blobMapList)
	{
		for (auto iter = blobMap.begin(); iter != blobMap.end(); ++iter)
		{
			iter->second->Release();
		}
	}

	_blobMapList.clear();
}

inline ShaderManager::BlobMap &ShaderManager::getBlobMap(const ShaderType type)
{
	return _blobMapList[EnumIndex(type)];
}
