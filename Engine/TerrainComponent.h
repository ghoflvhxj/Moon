#pragma once
#ifndef __TERRAIN_COMPONENT_H__

#include "Vertex.h"

#include "PrimitiveComponent.h"

class TextureComponent;
class VertexBuffer;
class IndexBuffer;
class MConstantBuffer;

class Material;

class ENGINE_DLL TerrainComponent : public PrimitiveComponent
{
public:
	using TileNum		= uint32;
	using TileInterval	= float;

	struct Data
	{
		Mat4 worldMatrix;
		Mat4 worldViewMatrix;
		Mat4 projectionMatrix;
	};

public:
	explicit TerrainComponent();
	explicit TerrainComponent(const TileNum x, const TileNum y);
	explicit TerrainComponent(const TileNum x, const TileNum y, const TileInterval interval);
	virtual ~TerrainComponent();

private:
	void initializeMeshInfromation();
private:
	std::vector<Vertex> _vertexList;
	std::vector<Index>	_indexList;


//public:
//	// PrimitiveComponent을(를) 통해 상속됨
//	virtual void render() override;

public:
	const TileNum getTileX() const;
	const TileNum getTileY() const;
private:
	TileNum _tileNumX;
	TileNum _tileNumY;

public:
	const TileInterval getTileInterval() const;
public:
	const TileInterval _interval;

public:
	const bool Test(const Vec3 &pos, float *pY);

public:
	const bool addTexture(std::shared_ptr<TextureComponent> pTexture);
	void setTexture(const uint32 index, std::shared_ptr<TextureComponent> pTexture);
protected:
	std::vector<std::shared_ptr<TextureComponent>> _textureList;

public:
	void setMaterial(std::shared_ptr<Material> pMaterial);
	std::shared_ptr<Material>& getMaterial();
private:
	std::shared_ptr<Material> _pMaterial;
};

#define __TERRAIN_COMPONENT_H__
#endif