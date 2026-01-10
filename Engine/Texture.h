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
	//void setTexture(const uint32 index = 0);

public:
    void SetTexture(ID3D11Texture2D* InTexture);
    ID3D11Texture2D* GetTexture();
protected:
    ID3D11Texture2D* Texture = nullptr;

public:
    void SetShaderResourceView(ID3D11ShaderResourceView* InSRVs);
    ID3D11ShaderResourceView* GetShaderResourceView();
protected:
    // 쉐이더에서 사용할 SRV 보관. 
    ID3D11ShaderResourceView* ShaderResurceView = nullptr;

public:
	const bool GetResolution(uint32& OutWidth, uint32& OutHeight);

public:
    REFLECT(
        MTexture,
    )
};