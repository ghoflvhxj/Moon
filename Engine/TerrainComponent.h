#pragma once
#ifndef __TERRAIN_COMPONENT_H__

#include "Vertex.h"

#include "PrimitiveComponent.h"

class MTexture;
class VertexBuffer;
class IndexBuffer;
class MConstantBuffer;

class MMaterial;

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
	const bool addTexture(std::shared_ptr<MTexture> pTexture);
	void setTexture(const uint32 index, std::shared_ptr<MTexture> pTexture);
protected:
	std::vector<std::shared_ptr<MTexture>> _textureList;

public:
	void setMaterial(std::shared_ptr<MMaterial> pMaterial);
	std::shared_ptr<MMaterial>& getMaterial();
private:
	std::shared_ptr<MMaterial> _pMaterial;
};

#define __TERRAIN_COMPONENT_H__
#endif