#pragma once

#include "Include.h"
#include "Vertex.h"
#include "Shader.h"

class MVertexBuffer;
class MIndexBuffer;
class MConstantBuffer;

class MShader;
class VertexShader;
class PixelShader;

class MTexture;
class MPrimitiveComponent;
class DynamicMeshComponent;

struct FShaderVariable;

class ENGINE_DLL MMaterial
{
public:
	explicit MMaterial();
	~MMaterial();

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
	std::shared_ptr<VertexShader>	_vertexShader;
	std::shared_ptr<PixelShader>	_pixelShader; 

public:
	void setTexture(const ETextureType textureType, std::shared_ptr<MTexture> pTexture);
	void setTextures(std::vector<std::shared_ptr<MTexture>> &textureList);
private:
	std::vector<std::shared_ptr<MTexture>> _textureList;

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
	std::vector<FShaderVariable>& getConstantBufferVariables(const ShaderType shaderType, const EConstantBufferLayer layer);
	std::vector<FShaderVariable>& getConstantBufferVariables(const ShaderType shaderType, const uint32 index);
	
	// 유틸리티	함수들
public:
	const bool IsTextureTypeUsed(const ETextureType type);

public:
    REFLECTABLE(
        REFLECT_FIELD(MMaterial, _vertexShaderFileName, EType::String),
        REFLECT_FIELD(MMaterial, _pixelShaderFileName, EType::String)
        //REFLECT_FIELD(MMaterial, _textureList, EType::Array)
    );
};