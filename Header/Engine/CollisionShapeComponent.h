#pragma once
#ifndef __COLLSION_SHAPE_COMPONENT_H__

#include "Vertex.h"

#include "PrimitiveComponent.h"

class Material;

class ENGINE_DLL CollisionShapeComponent : public PrimitiveComponent
{
public:
	explicit CollisionShapeComponent();
	explicit CollisionShapeComponent(std::vector<Vertex> &positionList);
	virtual ~CollisionShapeComponent();

private:
	void initialize();
	void initialize(std::vector<Vertex> &positionList);
private:
	std::vector<Vertex> _vertexList;
	std::vector<Index>	_indexList;
private:
	std::shared_ptr<Material> _pMaterial;

public:
	virtual void Update(const Time deltaTime) override;

//public:
//	virtual void render() override;
};

#define __COLLSION_SHAPE_COMPONENT_H__
#endif