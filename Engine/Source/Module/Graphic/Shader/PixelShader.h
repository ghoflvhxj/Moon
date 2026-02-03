#pragma once
#ifndef __PIXEL_SHADER_H__
#define __PIXEL_SHADER_H__

#include "Module/Graphic/Shader/Shader.h"

class MPixelShader : public MShader
{
public:
	explicit MPixelShader(const std::wstring &filePathName);
	explicit MPixelShader();
	virtual ~MPixelShader();

public:
	virtual void SetToDevice() override;

public:
	ID3D11PixelShader* getRaw();
private:
	ID3D11PixelShader *_pPixelShader;
};

#endif