#pragma once
#ifndef __PIXEL_SHADER_H__
#define __PIXEL_SHADER_H__

#include "Shader.h"
class PixelShader : public Shader
{
public:
	explicit PixelShader(const std::wstring &filePathName);
	explicit PixelShader();
	virtual ~PixelShader();

public:
	virtual void SetToDevice() override;

public:
	ID3D11PixelShader* getRaw();
private:
	ID3D11PixelShader *_pPixelShader;
};


#endif