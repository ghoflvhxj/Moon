#pragma once
#ifndef __VERTEX_SHADER_H__
#define __VERTEX_SHADER_H__

#include "Shader.h"

class VertexShader : public MShader
{
public:
	explicit VertexShader(const std::wstring &filePathName);
	explicit VertexShader();
	virtual ~VertexShader();

public:
	virtual void SetToDevice() override;

public:
	ID3D11VertexShader* getRaw();
private:
	ID3D11VertexShader *_pVertexShader;
};

#endif