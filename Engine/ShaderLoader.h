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
	bool loadShaderFiles(const std::unique_ptr<MShaderManager>& shaderManager);
private:
	bool loadVertexShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
	bool loadPixelShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
	bool loadGeometryShaderFromFiles(const std::unique_ptr<MShaderManager>& shaderManager);
};

#define __SHADER_LOADER_H__
#endif