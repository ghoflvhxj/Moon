#pragma once
#ifndef __MESH_COMPONENT_H__

#include "Vertex.h"

#include "PrimitiveComponent.h"

class MTexture;
class MMaterial;

class ENGINE_DLL MeshComponent abstract : public PrimitiveComponent
{
public:
	explicit MeshComponent();
	virtual ~MeshComponent();

// ���� ����
//private:
//	void initializeMeshInformation();
//private:
//	std::vector<Vertex> _vertexList;
//	std::vector<Index>	_indexList;

//public:
	//virtual void Update(const Time deltaTime) override;
	//virtual void render() override;

public:
	const bool addTexture(std::shared_ptr<MTexture> pTexture);
	void setTexture(const ETextureType textureType, std::shared_ptr<MTexture> pTexture);
	std::shared_ptr<MTexture>& getTexture(const ETextureType textureType);
private:
	std::vector<std::shared_ptr<MTexture>> _textureList;

public:
	void setMaterial(std::shared_ptr<MMaterial> pMaterial);
	std::shared_ptr<MMaterial>& getMaterial();
private:
	std::shared_ptr<MMaterial> _pMaterial;
};

#define __MESH_COMPONENT_H__
#endif