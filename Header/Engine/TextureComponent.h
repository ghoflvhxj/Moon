#pragma once
#ifndef __TEXTURE_COMPONENT_H__

#include "Component.h"

class ENGINE_DLL TextureComponent : public Component
{
public:
	using RawTexture	= ID3D11Texture2D;
	using RawTexturePtr = ID3D11Texture2D*;
	using ResourceView	= ID3D11ShaderResourceView;
	using ResourceViewPtr = ID3D11ShaderResourceView*;
public:
	explicit TextureComponent(const wchar_t *fileName);
	explicit TextureComponent(const char *fileName);
	explicit TextureComponent(RawTexturePtr pTexture);
	explicit TextureComponent(ID3D11ShaderResourceView *pShaderResourceView);
	explicit TextureComponent();
	virtual ~TextureComponent();

public:
	const bool loadTextureFile(const wchar_t *fileName);
	void setTexture(const uint32 index = 0);
	RawTexturePtr& getTextureRowPointer();
	ResourceViewPtr& getResourceViewRowPointer();
private:
	RawTexture		*_rawTexture;
	ResourceView	*_pResourceView;
};

#define __TEXTURE_COMPONENT_H__
#endif