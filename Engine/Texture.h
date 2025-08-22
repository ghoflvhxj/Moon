#pragma once

#include "Include.h"
#include "Core/Asset.h"

class ENGINE_DLL MTexture : public MAsset
{
public:
	explicit MTexture(const std::wstring& FilePath);
	explicit MTexture(ID3D11Texture2D* pTexture);
	explicit MTexture(ID3D11ShaderResourceView *pShaderResourceView);
    explicit MTexture(const MTexture& Rhs);
	explicit MTexture();
	virtual ~MTexture();

public:
    virtual bool Load(const std::wstring& InPath) override;
	const bool loadTextureFile(const wchar_t *fileName);
	void setTexture(const uint32 index = 0);
	ID3D11Texture2D*& GetTextureResource();
	ID3D11ShaderResourceView*& getRawResourceViewPointer();
private:
	ID3D11Texture2D*			_rawTexture;
	ID3D11ShaderResourceView*	_pResourceView;

public:
	const bool GetResolution(uint32& OutWidth, uint32& OutHeight);

public:
    REFLECT(
        MTexture,
    )
};