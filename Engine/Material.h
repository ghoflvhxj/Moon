#pragma once

#include "Include.h"
#include "Core/Asset.h"

#include "Vertex.h"
#include "Shader.h"
#include "Texture.h"

class MVertexBuffer;
class MIndexBuffer;
class MConstantBuffer;

class MShader;
class VertexShader;
class PixelShader;

//class MTexture;
class MPrimitiveComponent;
class DynamicMeshComponent;

struct FBufferVariable;

class ENGINE_DLL MMaterial : public MAsset
{
public:
	explicit MMaterial();
    MMaterial(const MMaterial& Rhs) = default;
	virtual ~MMaterial();

public:
    void SetName(const std::wstring& InName) { Name = InName; }
    std::wstring GetName() { return Name; }
protected:
    std::wstring Name;

public:
    virtual void OnLoaded() override;

public:
	void SetTexturesToDevice();

public:
	std::shared_ptr<MShader> getVertexShader();
	std::shared_ptr<MShader> getPixelShader();
	void setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName);
private:
	void ClearShader();
private:
	std::wstring _vertexShaderFileName;
	std::wstring _pixelShaderFileName;
	//std::shared_ptr<VertexShader>	_vertexShader;
	//std::shared_ptr<PixelShader>	_pixelShader; 

public:
	void setTexture(const ETextureType textureType, std::shared_ptr<MTexture> pTexture);
	void setTextures(std::vector<std::shared_ptr<MTexture>> &textureList);
private:
	std::vector<std::shared_ptr<MTexture>> _textureList;
    //std::vector<std::wstring> TexturePaths;

public:
	void setTopology(const D3D_PRIMITIVE_TOPOLOGY eTopology);
	void setFillMode(const Graphic::FillMode fillMode);
	void setCullMode(const Graphic::CullMode cullMode);
public:
	const D3D_PRIMITIVE_TOPOLOGY getTopology() const;
	const Graphic::FillMode getFillMode() const;
	const Graphic::CullMode getCullMode() const;
	const bool IsUseAlpha() const { return bUseAlpha; }
private:
	D3D_PRIMITIVE_TOPOLOGY _eTopology;
	Graphic::FillMode _eFillMode;
	Graphic::CullMode _eCullMode;
	bool bUseAlpha;

public:
	void SetAlphaMask(bool InValue) { bAlphaMask = InValue; }
	const bool IsAlphaMasked() const { return bAlphaMask; }
private:
	bool bAlphaMask;

public:
    bool IsRimLighted() const { return bRimLight; }
private:
    bool bRimLight = false;

public:
    void Test();
protected:
    bool bTest = false;



	// 유틸리티	함수들
public:
	const bool IsTextureTypeUsed(const ETextureType type);

public:
    REFLECT(
        MMaterial,
        PROPERTY(_vertexShaderFileName),
        PROPERTY(_pixelShaderFileName),
        PROPERTY(_textureList),
        PROPERTY(_eTopology),
        PROPERTY(_eFillMode),
        PROPERTY(_eCullMode),
        PROPERTY(bUseAlpha),
        PROPERTY(bAlphaMask)
        , PROPERTY(bRimLight)
    )
};
