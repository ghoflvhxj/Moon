#pragma once
#include "PrimitiveComponent.h"

#include "Core/Physics/PhysicsEnum.h"
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
    class PxMaterial;
    class PxDeformableSurface;
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
	const std::vector<::Vec3>& GetAllVertexPosition() const;
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
	std::shared_ptr<MBoundingBox> GetBoundingBox();
private:
	std::shared_ptr<MBoundingBox> _pBoundingBox;

public:
	const Vec3& GetCenterPos() const;
private:
	Vec3 CenterPos;
};

class ENGINE_DLL StaticMeshComponent : public MPrimitiveComponent
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
	virtual const bool GetBoundingBox(std::shared_ptr<MBoundingBox> &boundingBox) override;
	virtual void setTranslation(const Vec3 &translation) override;
	virtual void setScale(const Vec3& InScale) override;
    virtual Mat4& getWorldMatrix() override;
    Mat4 DeformalMatrix;

public:
	virtual XMMATRIX GetRotationMatrix();

public:
    void SetMesh(const std::wstring& Path);
	virtual std::shared_ptr<StaticMesh>& getStaticMesh();

private:
	std::shared_ptr<StaticMesh> _pStaticMesh = nullptr;

public:
	void Temp(float y);
	void SetGravity(bool bGravity);

    // 피직스 테스트용
public:
    void Clothing();

public:
	void SetMass(float NewMass);
    void SetAngularVelocity(float x, float y, float z);
    void SetVelocity(float x, float y, float z);

public:
    void SetPhysics(bool bInPhysics, bool bForce = false);
    void SetPhysicsSimulate(bool bInSimulate, bool bForce = false);
protected:
    bool bPhysics = true;
    bool bPhysicsSimulate = false;

public:
    void SetPhysicsType(EPhysicsType InPhysicsType) { PhysicsType = InPhysicsType; }
protected:
    EPhysicsType PhysicsType = EPhysicsType::Static;

	// 피직스
private:
	physx::PxConvexMesh*	PhysXConvexMesh = nullptr;
    physx::PxMaterial*      PhysxMaterial = nullptr;
    physx::PxDeformableSurface* DeformableSurface = nullptr;
    uint32 VertexNum = 0;
    
    // 피직스 렌더링을 위한 임시 메터리얼.
    std::shared_ptr<MMaterial> MaterialForPhysX = nullptr;
    // 피직스 렌더링을 위한 임시 메시데이터
    std::shared_ptr<FMeshData> MeshDataForPhysX = nullptr;

protected:
    std::shared_ptr<class MPhysicsObject> PhysicsObject;
};