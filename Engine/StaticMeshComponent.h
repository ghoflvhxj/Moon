#pragma once
#ifndef __STATIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "Vertex.h"

class VertexBuffer;
class IndexBuffer;

class ENGINE_DLL StaticMesh
{
public:
	StaticMesh() = default;
	
public: //삭제예정
	void initializeMeshInformation(const char *filePathName);
private:
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList >		_indicesList;
	std::vector<TextureList>	_textureList;

public:
	std::shared_ptr<Material> getMaterial(const uint32 index);
	const uint32 getMaterialCount() const;
private:
	MaterialList _materialList;


public:
	std::shared_ptr<VertexBuffer> getVertexBuffer();
	std::shared_ptr<IndexBuffer> getIndexBuffer();
private:
	std::shared_ptr<VertexBuffer> _pVertexBuffer = nullptr;
	std::shared_ptr<IndexBuffer> _pIndexBuffer = nullptr;

};

class ENGINE_DLL StaticMeshComponent : public PrimitiveComponent
{
public:
	explicit StaticMeshComponent();
	explicit StaticMeshComponent(const char *filePathName);
	virtual ~StaticMeshComponent();

public:
	virtual const bool getPrimitiveData(PrimitiveData &primitiveData) override;

public:
	virtual std::shared_ptr<StaticMesh>& getStaticMesh();

private:
	std::shared_ptr<StaticMesh> _pStaticMesh = nullptr;
};

#define __STATIC_MESH_COMPONENT_H__
#endif