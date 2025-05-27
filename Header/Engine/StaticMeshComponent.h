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

struct FMeshData
{
	VertexList		Vertices;
	IndexList 		Indices;
};

class ENGINE_DLL StaticMesh
{
public:
	StaticMesh() = default;
	
public:
	virtual void InitializeFromFBX(const std::wstring& Path);
public:
	const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
	const std::vector<Vec3>& GetAllVertexPosition() const;
	std::vector<TextureList>	Tetures;
	std::vector<uint32>			_geometryLinkMaterialIndices;
	std::vector<Vec3>			AllVertexPosition;

public:
	std::weak_ptr<FMeshData> GetMeshData(const uint32 Index) const { return MeshDataList[Index]; }
protected:
	std::vector<std::shared_ptr<FMeshData>> MeshDataList;

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

public:
	const Vec3& GetCenterPos() const;
private:
	Vec3 CenterPos;
};

class ENGINE_DLL StaticMeshComponent : public PrimitiveComponent
{
public:
	using SceneComponent::setTranslation;
	using SceneComponent::setScale;

public:
	explicit StaticMeshComponent();
	explicit StaticMeshComponent(const char *filePathName);
	explicit StaticMeshComponent(const char *filePathName, bool bUsePhysX, bool bUseRigidStatic = true);
	virtual ~StaticMeshComponent();

public:
	virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;
	virtual const bool getBoundingBox(std::shared_ptr<BoundingBox> &boundingBox) override;
	virtual void setTranslation(const Vec3 &translation) override;
	virtual void setScale(const Vec3 &scale) override;

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

public:
	void SetStaticCollision(bool bNewCollision, bool bForce = false);
	void SetMass(float NewMass);
private:
	bool bStaticCollision = true;
	float Mass = 1.f;

	// ÇÇÁ÷½º
public:
	physx::PxRigidActor*	GetPhysXRigidActor();
	physx::PxRigidDynamic*	GetPhysXRigidDynamic();
	physx::PxRigidStatic*	GetPhysXRigidStatic();
	physx::PxShape*			GetPhysXShape();
private:
	physx::PxConvexMesh*	PhysXConvexMesh = nullptr;
	physx::PxRigidStatic*	PhysXRigidStatic = nullptr;
	physx::PxRigidDynamic*	PhysXRigidDynamic = nullptr;
	physx::PxShape*			PhysXShape = nullptr;
};

#define __STATIC_MESH_COMPONENT_H__
#endif