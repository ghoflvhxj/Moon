#pragma once

#include "Include.h"

#include "Shader.h"

class MShader;
class VertexShader;
class PixelShader;
class MGeometryShader;

class MShaderManager
{
	using ShaderMap				= std::unordered_map<std::wstring, std::shared_ptr<MShader>>;
	using ShaderReflectionMap	= std::unordered_map<std::wstring, ID3D11ShaderReflection*>;
	using BlobMap				= std::unordered_map<std::wstring, ID3D10Blob*>;

public:
	explicit MShaderManager();
	~MShaderManager() = default;

public:		
	const bool						getVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader);
	const bool						addVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader);
	const bool						getPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader);
	const bool						addPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader);
	const bool						getGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader);
	const bool						addGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader);
private:
	const bool						addShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &pShader);
	ShaderMap&						getShaderMap(const ShaderType type);
	const bool 						getShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &pShader);

public:
	MShaderManager::ShaderMap&		GetShaders(const ShaderType shaderType);
private:
	std::vector<ShaderMap> _shadersPerShaderType;

public:
    // InputLayout 생성 시 사용할 함수
    ID3D10Blob*						getVertexShaderBlob(const wchar_t *shaderName);
};
