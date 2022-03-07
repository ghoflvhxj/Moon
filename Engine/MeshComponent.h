#pragma once
#ifndef __MESH_COMPONENT_H__

#include "Vertex.h"

#include "PrimitiveComponent.h"

class TextureComponent;
class Material;

class ENGINE_DLL MeshComponent /* abstract */ : public PrimitiveComponent
{
public:
	explicit MeshComponent();
	virtual ~MeshComponent();

private:
	void initializeMeshInformation();
private:
	std::vector<Vertex> _vertexList;
	std::vector<Index>	_indexList;

public:
	//virtual void Update(const Time deltaTime) override;
	virtual void render() override;

public:
	const bool addTexture(std::shared_ptr<TextureComponent> pTexture);
	void setTexture(const TextureType textureType, std::shared_ptr<TextureComponent> pTexture);
	std::shared_ptr<TextureComponent>& getTexture(const TextureType textureType);
private:
	std::vector<std::shared_ptr<TextureComponent>> _textureList;

public:
	void setMaterial(std::shared_ptr<Material> pMaterial);
	std::shared_ptr<Material>& getMaterial();
private:
	std::shared_ptr<Material> _pMaterial;
};

#define __MESH_COMPONENT_H__
#endif