#pragma once

#include "Include.h"

#include "Module/Graphic/Shader/Shader.h"
#include "Module/Graphic/Shader/ComputeShader.h"

class MShader;
class MVertexShader;
class MPixelShader;
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
	const bool						getVertexShader(const wchar_t *fileName, std::shared_ptr<MVertexShader> &vertexShader);
	const bool						addVertexShader(const wchar_t *fileName, std::shared_ptr<MVertexShader> &vertexShader);
	const bool						getPixelShader(const wchar_t *fileName, std::shared_ptr<MPixelShader> &pixelShader);
	const bool						addPixelShader(const wchar_t *fileName, std::shared_ptr<MPixelShader> &pixelShader);
	const bool						getGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader);
	const bool						addGeometryShader(const wchar_t *fileName, std::shared_ptr<MGeometryShader> &geometryShader);
    void AddComputeShader(const std::wstring& InPath, const MComputeShader& InComputeShader);
    const MComputeShader& GetComputeShader(const std::wstring& InFileName);
private:
	const bool						addShader(const EShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &pShader);
	ShaderMap&						getShaderMap(const EShaderType type);
	const bool 						getShader(const EShaderType type, const wchar_t *fileName, std::shared_ptr<MShader> &pShader);

public:
	MShaderManager::ShaderMap&		GetShaders(const EShaderType shaderType);
private:
	std::vector<ShaderMap> _shadersPerShaderType;
    
    std::map<std::wstring, MComputeShader> ComputeShaders;

public:
    // InputLayout 생성 시 사용할 함수
    ID3D10Blob*						getVertexShaderBlob(const wchar_t *shaderName);
};
