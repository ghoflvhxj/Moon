#include "Include.h"
#include "ShaderManager.h"

#include "Module/Graphic/Shader/Shader.h"
#include "Module/Graphic/Shader/VertexShader.h"
#include "Module/Graphic/Shader/PixelShader.h"
#include "Module/Graphic/Shader/GeometryShader.h"

#include "ShaderLoader.h"

#include "MapUtility.h"

MShaderManager::MShaderManager()
	: _shadersPerShaderType(EnumToIndex(EShaderType::Count), ShaderMap())
{
}

const bool MShaderManager::addShader(const EShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &pShader)
{
	if (false == MapUtility::FindInsert(getShaderMap(type), fileName, pShader))
	{
		return false;
	}

	return true;
}

MShaderManager::ShaderMap &MShaderManager::getShaderMap(const EShaderType type)
{
	return _shadersPerShaderType[EnumToIndex(type)];
}

const bool MShaderManager::getShader(const EShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &shader)
{
	if (false == MapUtility::FindGet(getShaderMap(type), std::wstring(fileName), shader))
	{
		DEV_ASSERT_MSG("쉐이더 파일을 찾을 수 없습니다!");
		return false;
	}

	return true;
}

const bool MShaderManager::getVertexShader(const wchar_t *fileName, std::shared_ptr<MVertexShader> &vertexShader)
{
	std::shared_ptr<MShader> pShader = nullptr;
	if (true == getShader(EShaderType::Vertex, fileName, pShader))
	{
		vertexShader = std::static_pointer_cast<MVertexShader>(pShader);
		return true;
	}

	return false;
}

const bool MShaderManager::addVertexShader(const wchar_t *fileName, std::shared_ptr<MVertexShader> &vertexShader)
{
	std::shared_ptr<MShader> pShader = vertexShader;
	return addShader(EShaderType::Vertex, fileName, pShader);
}

const bool MShaderManager::getPixelShader(const wchar_t *fileName, std::shared_ptr<MPixelShader> &pixelShader)
{
	if (fileName == nullptr)
	{
		return false;
	}

	std::shared_ptr<MShader> pShader = nullptr;
	if (true == getShader(EShaderType::Pixel, fileName, pShader))
	{
		pixelShader = std::static_pointer_cast<MPixelShader>(pShader);
		return true;
	}

	return false;
}

const bool MShaderManager::addPixelShader(const wchar_t *fileName, std::shared_ptr<MPixelShader> &pixelShader)
{
	std::shared_ptr<MShader> pShader = pixelShader;
	return addShader(EShaderType::Pixel, fileName, pShader);
}

const bool MShaderManager::getGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader)
{
	std::shared_ptr<MShader> pShader = nullptr;
	if (true == getShader(EShaderType::Geometry, fileName, pShader))
	{
		geometryShader = std::static_pointer_cast<MGeometryShader>(pShader);
		return true;
	}

	return false;
}

const bool MShaderManager::addGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader)
{
	std::shared_ptr<MShader> pShader = geometryShader;
	return addShader(EShaderType::Geometry, fileName, pShader);
}

void MShaderManager::AddComputeShader(const std::wstring& InPath, const MComputeShader& InComputeShader)
{
    ComputeShaders[MFileSystem::GetFileName(InPath)] = InComputeShader;
    //addShader(EShaderType::Compute, MFileSystem::GetFileName(InPath).c_str(), pShader);
}

const MComputeShader& MShaderManager::GetComputeShader(const std::wstring& InFileName)
{
    assert(ComputeShaders.find(InFileName) != ComputeShaders.end());

    return ComputeShaders[InFileName];
}

MShaderManager::ShaderMap& MShaderManager::GetShaders(const EShaderType shaderType)
{
	return _shadersPerShaderType[CastValue<uint32>(shaderType)];
}

ID3D10Blob *MShaderManager::getVertexShaderBlob(const wchar_t *shaderName)
{
	std::shared_ptr<MVertexShader> pShader = nullptr;
	if (true == getVertexShader(shaderName, pShader))
	{
		return pShader->getBlob();
	}

	return nullptr;
}