#pragma once
#ifndef __SHADER_MANAGER_H__

#include "Shader.h"

class Shader;
class VertexShader;
class PixelShader;
class GeometryShader;

class ShaderManager
{
	using ShaderMap				= std::unordered_map<std::wstring, std::shared_ptr<Shader>>;
	using ShaderReflectionMap	= std::unordered_map<std::wstring, ID3D11ShaderReflection*>;
	using BlobMap				= std::unordered_map<std::wstring, ID3D10Blob*>;

public:
	explicit ShaderManager();
	~ShaderManager();

public:
	void							Release();

public:		
	const bool						getVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader);
	const bool						addVertexShader(const wchar_t *fileName, std::shared_ptr<VertexShader> &vertexShader);
	const bool						getPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader);
	const bool						addPixelShader(const wchar_t *fileName, std::shared_ptr<PixelShader> &pixelShader);
	const bool						getGeometryShader(const wchar_t *fileName, std::shared_ptr<GeometryShader> &geometryShader);
	const bool						addGeometryShader(const wchar_t *fileName, std::shared_ptr<GeometryShader> &geometryShader);
private:
	const bool						addShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<Shader> &pShader);
	ShaderMap&						getShaderMap(const ShaderType type);
	const bool 						getShader(const ShaderType type, const wchar_t *fileName, std::shared_ptr<Shader> &pShader);

public:
	ShaderManager::ShaderMap&		getShaders(const ShaderType shaderType);
private:
	std::vector<ShaderMap> _shadersPerShaderType;

public:
	ID3D10Blob*						getVertexShaderBlob(const wchar_t *shaderName);
//	void							releaseBlob();	// InputLayout 생성을 위해 남겨두었던 Blob을 소멸시킵니다.
//public:
//	inline BlobMap&					getBlobMap(const ShaderType type);
//private:
//	std::vector<BlobMap> _blobMapList;
//
//public:
//	ShaderReflectionMap&			getReflectionMap(const ShaderType type);
//private:
//	std::vector<ShaderReflectionMap> _reflectionMapList;
};

#define __SHADER_MANAGER_H__
#endif