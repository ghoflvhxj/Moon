#pragma once
#ifndef __TEXTURE_COMPONENT_H__

#include "Component.h"

class ENGINE_DLL TextureComponent : public Component
{
public:
	explicit TextureComponent(const wchar_t *fileName);
	explicit TextureComponent(const char *fileName);
	explicit TextureComponent(ID3D11Texture2D *pTexture);
	explicit TextureComponent(ID3D11ShaderResourceView *pShaderResourceView);
	//explicit TextureComponent();
	virtual ~TextureComponent();

public:
	const bool loadTextureFile(const wchar_t *fileName);
	void setTexture(const uint32 index = 0);
private:
	ID3D11Texture2D				*_pTexture;
	ID3D11ShaderResourceView	*_pResourceView;
};

#define __TEXTURE_COMPONENT_H__
#endif