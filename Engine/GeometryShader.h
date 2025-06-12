#pragma once
#ifndef __GEOMETRY_SHADER_H__
#define __GEOMETRY_SHADER_H__

#include "Shader.h"

class MGeometryShader : public MShader
{
public:
	explicit MGeometryShader(const std::wstring &filePathName);
	explicit MGeometryShader();
	virtual ~MGeometryShader();

public:
	virtual void SetToDevice() override;

public:
	ID3D11GeometryShader* getRaw();
private:
	ID3D11GeometryShader *_pGeometryShader;
};

#endif