#pragma once
#ifndef __STATIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "Vertex.h"

class MVertexBuffer;
class MIndexBuffer;
class MFBXLoader;

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
	void LoadFromFBX(const std::wstring& Path);
	virtual void InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath);
public:
	const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
	const std::vector<Vec3>& GetAllVertexPosition() const;
	std::vector<TextureList>	Textures;
	std::vector<uint32>			_geometryLinkMaterialIndices;
	std::vector<Vec3>			AllVertexPosition;

public:
	const std::shared_ptr<FMeshData>& GetMeshData(const uint32 Index) const;
	const uint32 GetMeshNum() const { return static_cast<uint32>(MeshDataList.size()); }
protected:
	std::vector<std::shared_ptr<FMeshData>> MeshDataList;

public:
	MaterialList& getMaterials();
	std::shared_ptr<MMaterial> getMaterial(const uint32 index);
	const uint32 GetMaterialNum() const;
protected:
	MaterialList Materials;

public:
	const uint32 getGeometryCount() const;
protected:
	uint32 GeometryNum = 0;

public:
	const uint32 getVertexCount() const;
	uint32 TotalVertexNum = 0;

public:
	std::shared_ptr<BoundingBox> getBoundingBox();
private:
	std::shared_ptr<BoundingBox> _pBoundingBox;

public:
	std::vector<std::shared_ptr<MVertexBuffer>> getVertexBuffers();
	std::shared_ptr<MIndexBuffer> getIndexBuffer();
protected:
	std::vector<std::shared_ptr<MVertexBuffer>> _pVertexBuffers;
	std::shared_ptr<MIndexBuffer> _pIndexBuffer = nullptr;

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
	explicit StaticMeshComponent(const std::wstring& FilePath);
	explicit StaticMeshComponent(const std::wstring& FilePath, bool bUsePhysX, bool bUseRigidStatic = true);
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
	void Temp(float y);
	void SetGravity(bool bGravity);

public:
	void SetStaticCollision(bool bNewCollision, bool bForce = false);
	void SetMass(float NewMass);
private:
	bool bStaticCollision = true;
	float Mass = 1.f;

	// 피직스
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