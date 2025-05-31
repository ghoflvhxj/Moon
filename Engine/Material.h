#pragma once
#ifndef __MATERIAL_H__

#include "Vertex.h"
#include "Shader.h"

class VertexBuffer;
class IndexBuffer;
class MConstantBuffer;

class MShader;
class VertexShader;
class PixelShader;

class TextureComponent;
class PrimitiveComponent;
class DynamicMeshComponent;

struct FShaderVariable;

class ENGINE_DLL Material
{
public:
	explicit Material();
	~Material();

public:
	void SetTexturesToDevice();

	// 삭제 예정
public:
	void setOwner(std::shared_ptr<PrimitiveComponent> pOwner);
private:
	std::shared_ptr<PrimitiveComponent> _pOwner;

public:
	std::shared_ptr<MShader> getVertexShader();
	std::shared_ptr<MShader> getPixelShader();
	void setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName);
private:
	void releaseShader();
private:
	std::wstring _vertexShaderFileName;
	std::wstring _pixelShaderFileName;
	std::shared_ptr<VertexShader>	_vertexShader;
	std::shared_ptr<PixelShader>	_pixelShader; 

public:
	void setTexture(const TextureType textureType, std::shared_ptr<TextureComponent> pTexture);
	void setTexture(std::vector<std::shared_ptr<TextureComponent>> &textureList);
private:
	std::vector<std::shared_ptr<TextureComponent>> _textureList;

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
	std::vector<FShaderVariable>& getConstantBufferVariableInfos(const ShaderType shaderType, const EConstantBufferLayer layer);
	std::vector<FShaderVariable>& getConstantBufferVariableInfos(const ShaderType shaderType, const uint32 index);
private:
	std::vector<std::vector<std::vector<FShaderVariable>>> _variableInfosPerShaderType;	// 각 쉐이더 당 콘스탄트 버퍼의 변수 정보 저장
	

public:
	const bool useTextureType(const TextureType type);
//public:
//	void setFillMode(const Graphic::FillMode eFillMode);
//private:
//	Graphic::FillMode _eFillMode;
//
//public:
//	void setCullMode(const Graphic::CullMode eCullMode);
//private:
//	Graphic::CullMode _eCullMode;
//
//public:
//	void setDepthWriteMode(const Graphic::DepthWriteMode eDepthWriteMode);
//private:
//	Graphic::DepthWriteMode _eDepthWriteMode;
//
//public:
//	void setBlendState(const Graphic::Blend eBlend);
//private:
//	Graphic::Blend _eBlend;
};

#define __MATERIAL_H__
#endif