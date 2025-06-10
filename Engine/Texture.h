#pragma once

#include "Core/ResourceLoader.h"

class ENGINE_DLL MTexture : public MResource
{
public:
	explicit MTexture(const std::wstring& FilePath);
	explicit MTexture(ID3D11Texture2D* pTexture);
	explicit MTexture(ID3D11ShaderResourceView *pShaderResourceView);
	explicit MTexture();
	virtual ~MTexture();

public:
	const bool loadTextureFile(const wchar_t *fileName);
	void setTexture(const uint32 index = 0);
	ID3D11Texture2D*& GetTextureResource();
	ID3D11ShaderResourceView*& getRawResourceViewPointer();
private:
	ID3D11Texture2D*			_rawTexture;
	ID3D11ShaderResourceView*	_pResourceView;

public:
	const bool GetResolution(uint32& OutWidth, uint32& OutHeight);
};