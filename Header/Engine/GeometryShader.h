#pragma once
#ifndef __GEOMETRY_SHADER_H__
#define __GEOMETRY_SHADER_H__

#include "Shader.h"

class GeometryShader : public Shader
{
public:
	explicit GeometryShader(const std::wstring &filePathName);
	explicit GeometryShader();
	virtual ~GeometryShader();

public:
	virtual void SetToDevice() override;

public:
	ID3D11GeometryShader* getRaw();
private:
	ID3D11GeometryShader *_pGeometryShader;
};

#endif