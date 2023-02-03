#pragma once
#ifndef __STATIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "Vertex.h"

class VertexBuffer;
class IndexBuffer;

namespace physx
{
	class PxConvexMesh;
	class PxActor;
	class PxRigidBody;
	class PxRigidStatic;
	class PxRigidDynamic;
	class PxRigidActor;
	class PxShape;
};

class ENGINE_DLL StaticMesh
{
public:
	StaticMesh() = default;
	
public:
	virtual void initializeMeshInformation(const char *filePathName, bool bUsePhysX = true, bool bStaticBody = true);
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
	const uint32 getVertexCount() const;
	uint32 VertexCount = 0;

public:
	std::shared_ptr<BoundingBox> getBoundingBox();
private:
	std::shared_ptr<BoundingBox> _pBoundingBox;

public:
	std::vector<std::shared_ptr<VertexBuffer>> getVertexBuffers();
	std::shared_ptr<IndexBuffer> getIndexBuffer();
protected:
	std::vector<std::shared_ptr<VertexBuffer>> _pVertexBuffers;
	std::shared_ptr<IndexBuffer> _pIndexBuffer = nullptr;

	// ÇÇÁ÷½º
public:
	physx::PxRigidActor* GetPhysXActor();
	physx::PxRigidDynamic* GetPhysXRigidDynamic();
	physx::PxRigidStatic* GetPhysXRigidStatic();
private:
	physx::PxConvexMesh* PhysXConvexMesh = nullptr;
	physx::PxRigidActor* PhysxRigidActor = nullptr;
	physx::PxShape*				PhysXShape = nullptr;

	bool bStatic = true;
public:
	Vec3 CenterPos;
};

class ENGINE_DLL StaticMeshComponent : public PrimitiveComponent
{
public:
	explicit StaticMeshComponent();
	explicit StaticMeshComponent(const char *filePathName);
	explicit StaticMeshComponent(const char *filePathName, bool bUsePhysX, bool bRigidStatic = true);
	virtual ~StaticMeshComponent();

public:
	virtual void Update(const Time deltaTime) override;
	virtual const bool getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList) override;
	virtual const bool getBoundingBox(std::shared_ptr<BoundingBox> &boundingBox) override;

public:
	virtual XMMATRIX GetRotationMatrix();

public:
	virtual std::shared_ptr<StaticMesh>& getStaticMesh();

private:
	std::shared_ptr<StaticMesh> _pStaticMesh = nullptr;

public:
	void setDrawingBoundingBox(const bool bDraw);
	const bool IsDrawingBoundingBox() const;
public:
	bool _bDrawBoundingBox = false;

public:
	void Temp(float y);
	void SetGravity(bool bGravity);
};

#define __STATIC_MESH_COMPONENT_H__
#endif