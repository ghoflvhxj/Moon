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
	const bool loadShaderFiles(std::shared_ptr<MShaderManager> shaderManager);
private:
	const bool loadVertexShaderFromFiles(std::shared_ptr<MShaderManager> shaderManager);
	const bool loadPixelShaderFromFiles(std::shared_ptr<MShaderManager> shaderManager);
	const bool loadGeometryShaderFromFiles(std::shared_ptr<MShaderManager> shaderManager);
};

#define __SHADER_LOADER_H__
#endif