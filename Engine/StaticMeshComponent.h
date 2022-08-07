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
	virtual void initializeMeshInformation(const char *filePathName);
public:
	const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
protected:
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList >		_indicesList;
	std::vector<TextureList>	_textureList;
	std::vector<uint32>			_geometryLinkMaterialIndices;

public:
	MaterialList& getMaterials();
	std::shared_ptr<Material> getMaterial(const uint32 index);
	const uint32 getMaterialCount() const;
protected:
	MaterialList _materialList;

public:
	const uint32 getGeometryCount() const;
protected:
	uint32 _geometryCount = 0;

public:
	std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers();
	std::shared_ptr<IndexBuffer> getIndexBuffer();
protected:
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
	virtual const bool getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList) override;

public:
	virtual std::shared_ptr<StaticMesh>& getStaticMesh();

private:
	std::shared_ptr<StaticMesh> _pStaticMesh = nullptr;
};

#define __STATIC_MESH_COMPONENT_H__
#endif