#pragma once
#ifndef __SHADER_LOADER_H__

class MShaderManager;
class FileFinder;

class ShaderLoader
{
public:
	explicit ShaderLoader();
	~ShaderLoader();

public:
	const bool loadShaderFiles(const std::unique_ptr<MShaderManager>& shaderManager);
private:
	const bool loadVertexShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
	const bool loadPixelShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
	const bool loadGeometryShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
};

#define __SHADER_LOADER_H__
#endif