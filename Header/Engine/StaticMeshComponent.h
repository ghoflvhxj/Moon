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
public:
	const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
private:
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList >		_indicesList;
	std::vector<TextureList>	_textureList;
	std::vector<uint32>			_geometryLinkMaterialIndices;

public:
	MaterialList getMaterials() const;
	std::shared_ptr<Material> getMaterial(const uint32 index);
	const uint32 getMaterialCount() const;
private:
	MaterialList _materialList;


public:
	std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers();
	std::shared_ptr<IndexBuffer> getIndexBuffer();
private:
	std::vector<std::shared_ptr<VertexBuffer>> _pVertexBuffers;
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