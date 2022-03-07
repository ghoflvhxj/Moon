#pragma once
#ifndef __COLLISION_RENDERER_H__

#include "Vertex.h"

class StaticMeshComponent;

class CollisionRenderer
{
	using BoxList = std::vector<int>;

public:
	explicit CollisionRenderer();
	~CollisionRenderer();

private:
	void initializeMeshInformation();
private:
	std::vector<Vertex> _vertexList;
	std::vector<Index>	_indexList;

public:
	void render();

public:
	void makeBox();
public:
	BoxList _boxList;

//private:
//	std::shared_ptr<Material> _pMaterial;
};

#define __COLLISION_RENDERER_H__
#endif