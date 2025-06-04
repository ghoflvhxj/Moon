#pragma once
#ifndef __TEXTURE_COMPONENT_H__

class ENGINE_DLL MTexture
{
public:
	using RawTexture	= ID3D11Texture2D;
	using RawTexturePtr = ID3D11Texture2D*;
	using ResourceView	= ID3D11ShaderResourceView;
	using ResourceViewPtr = ID3D11ShaderResourceView*;
public:
	explicit MTexture(const wchar_t *fileName);
	explicit MTexture(const char *fileName);
	explicit MTexture(RawTexturePtr pTexture);
	explicit MTexture(ID3D11ShaderResourceView *pShaderResourceView);
	explicit MTexture();
	virtual ~MTexture();

public:
	const bool loadTextureFile(const wchar_t *fileName);
	void setTexture(const uint32 index = 0);
	RawTexturePtr& getRawTexturePointer();
	ResourceViewPtr& getRawResourceViewPointer();
private:
	RawTexture		*_rawTexture;
	ResourceView	*_pResourceView;
};

#define __TEXTURE_COMPONENT_H__
#endif