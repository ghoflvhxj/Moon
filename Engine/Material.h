#pragma once
#ifndef __MATERIAL_H__

#include "Vertex.h"

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

class TextureComponent;
class PrimitiveComponent;

class ENGINE_DLL Material
{
	using VertexBufferLis	= std::vector<std::shared_ptr<VertexBuffer>>;
	using IndexBufferLis	= std::vector<std::shared_ptr<IndexBuffer>>;

	__declspec(align(16)) struct VertexShaderConstantBuffer
	{
		Mat4 worldMatrix;
		Mat4 cameraViewMatrix;
		Mat4 projectionMatrix;
	};

	__declspec(align(16)) struct PixelShaderConstantBuffer
	{
		BOOL usingNormalTexture;
		BOOL usingSepcuarTexture;
	};

public:
	explicit Material(std::vector<Vertex> &vertexList);
	explicit Material(std::vector<Vertex> &vertexList, std::vector<Index> &indexList);
	explicit Material(std::vector<VertexList> &verticesList);
	explicit Material(std::vector<VertexList> &verticesList, std::vector<IndexList> &indicesList);
	explicit Material() = delete;
	~Material();
private:
	std::vector<uint32>	_vertexOffsetList;
	std::vector<uint32>	_indexOffsetList;
private:
	void initializeVertexListFromVerticesList(VertexList &vertexList, std::vector<VertexList> &verticesList);
	void initializeIndexListFromIndicesList(IndexList &indexList, std::vector<IndexList> &IndicesList);
	void initializeBuffers(std::vector<Vertex> &vertexList);
	void initializeBuffers(std::vector<Vertex> &vertexList, std::vector<Index> &indexList);
	void initializeConstantBuffers();
private:
	std::shared_ptr<VertexBuffer>	_pVertexBuffer;
	std::shared_ptr<IndexBuffer>	_pIndexBuffer;
	std::shared_ptr<ConstantBuffer> _pVertexConstantBuffer;
	std::shared_ptr<ConstantBuffer> _pPixelConstantBuffer;

	// 삭제 예정
public:
	void setOwner(std::shared_ptr<PrimitiveComponent> pOwner);
private:
	std::shared_ptr<PrimitiveComponent> _pOwner;

public:
	void setShader(const wchar_t *vertexShader, const wchar_t *pixelShader);
private:
	ID3D11VertexShader *_pVertexShader;
	ID3D11PixelShader *_pPixelShader;

public:
	void render(std::shared_ptr<PrimitiveComponent> pComponent);

public:
	void setTexture(const TextureType textureType, std::shared_ptr<TextureComponent> pTexture);
	void setTexture(std::vector<std::shared_ptr<TextureComponent>> &textureList);
private:
	std::vector<std::shared_ptr<TextureComponent>> _textureList;

public:
	void setTopology(const D3D_PRIMITIVE_TOPOLOGY eTopology);
private:
	D3D_PRIMITIVE_TOPOLOGY _eTopology;

public:
	void setFillMode(const Graphic::FillMode eFillMode);
private:
	Graphic::FillMode _eFillMode;

public:
	void setCullMode(const Graphic::CullMode eCullMode);
private:
	Graphic::CullMode _eCullMode;

public:
	void setDepthWriteMode(const Graphic::DepthWriteMode eDepthWriteMode);
private:
	Graphic::DepthWriteMode _eDepthWriteMode;

public:
	void setBlendState(const Graphic::Blend eBlend);
private:
	Graphic::Blend _eBlend;
};

#define __MATERIAL_H__
#endif