#pragma once
#ifndef __VERTEX_SHADER_H__
#define __VERTEX_SHADER_H__

#include "Shader.h"

class VertexShader : public Shader
{
public:
	explicit VertexShader(const std::wstring &filePathName);
	virtual ~VertexShader();

public:
	virtual void SetToDevice() override;

public:
	ID3D11VertexShader* getRaw();
private:
	ID3D11VertexShader *_pVertexShader;

	std::wstring fileName;
};

#endif