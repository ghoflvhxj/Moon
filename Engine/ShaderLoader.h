#pragma once
#ifndef __SHADER_LOADER_H__

class ShaderManager;
class FileFinder;

class ShaderLoader
{
public:
	explicit ShaderLoader();
	~ShaderLoader();

public:
	const bool loadShader(std::shared_ptr<ShaderManager> shaderManager);
private:
	const bool loadVertexShaderFromFiles(std::shared_ptr<ShaderManager> shaderManager);
	const bool loadPixelShaderFromFiles(std::shared_ptr<ShaderManager> shaderManager);
	const bool extractConstantBuffer(ID3D10Blob *pBlob);
public:
	void clear();
	void clearShader();
	void clearRelease();
};

#define __SHADER_LOADER_H__
#endif