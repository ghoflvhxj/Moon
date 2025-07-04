#pragma once

#include "Include.h"

class ENGINE_DLL MTexture : public MResource
{
public:
	explicit MTexture(const std::wstring& FilePath);
	explicit MTexture(ID3D11Texture2D* pTexture);
	explicit MTexture(ID3D11ShaderResourceView *pShaderResourceView);
    explicit MTexture(const MTexture& Rhs);
	explicit MTexture();
	virtual ~MTexture();

public:
    // 매터리얼json을 로드할 때, 텍스쳐 파일만 세팅되어 있음
    void Load();
	const bool loadTextureFile(const wchar_t *fileName);
	void setTexture(const uint32 index = 0);
	ID3D11Texture2D*& GetTextureResource();
	ID3D11ShaderResourceView*& getRawResourceViewPointer();
private:
	ID3D11Texture2D*			_rawTexture;
	ID3D11ShaderResourceView*	_pResourceView;

public:
	const bool GetResolution(uint32& OutWidth, uint32& OutHeight);

protected:
    std::wstring Path;

public:
    REFLECTABLE(
        REFLECT_FIELD(MTexture, Path)
    )
};