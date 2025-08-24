#pragma once

#include "Include.h"

#include "Core/Asset.h"
#include "Core/Module/Module.h"

#include "PhysicsEnum.h"
#include "Vertex.h"
#include "Mesh/Mesh.h" // FClothData 참조

class StaticMesh;
class MPhysicsObject;
class MPrimitiveComponent;
class MMeshComponent;

struct FPhysicsConstructData
{
	std::shared_ptr<MPrimitiveComponent> PrimitiveComponent;
	std::shared_ptr<StaticMesh> Mesh;
	EPhysicsType PhysicsType;
    ::Vec3 Pos = VEC3ZERO;
    // 임시
    bool bCapsule = false;
};

class ENGINE_DLL MPhysics : public MAsset
{
    REFLECT(MPhysics)
};

class ENGINE_DLL MPhysicsEngine : public MModule
{
public:
    MPhysicsEngine() = default;
    virtual ~MPhysicsEngine() = default;

public:
    virtual void LoadTest() {}
    virtual void SaveTest(std::shared_ptr<StaticMesh> Mesh) {}

public:
    virtual void StartSimulate();
    virtual void Update() {}
    virtual void Release();

public:
    virtual void AddMeshComponent(std::shared_ptr<MMeshComponent> InMeshComp);
    virtual void MakeConvexHull(FPhysicsConstructData& InData) {}

public:
    // Component를 받도록 변경
    virtual bool AddPhysicsObject(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) = 0;
    virtual bool AddCloth(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) { return false; }
    virtual void AddCloth(FPhysicsConstructData& InData, std::vector<FClothData>& ClothData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) {}

protected:
    std::vector<std::weak_ptr<MPhysicsObject>> SoftBodyObjects;

    // 프리미티브ID - 피직스 오브젝트 쌍
    std::map<uint32, std::shared_ptr<MPhysicsObject>> PhysicsObjects;

protected:
    bool bSimulating = false;

    // 임시. 시뮬레이션 등록 컴포넌트
    std::vector<std::weak_ptr<MMeshComponent>> MeshComponents;

    REFLECT(MPhysicsEngine)
};

class ENGINE_DLL MPhysicsObject
{
public:
	MPhysicsObject(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent, std::shared_ptr<StaticMesh> InMesh)
		: Owner(InPrimitiveComponent)
		, MeshCache(InMesh)
    {

    }
    virtual ~MPhysicsObject() = default;

public:
    virtual void UpdateVertices(std::vector<Graphic::VERTEX_COMMON>& InVertices) {}
    virtual void MoveTo(const ::Vec3& TargetPos) {}
    virtual void Remove() {}

public:
    virtual bool IsSimulating() = 0;
	virtual bool IsSoftBody() { return false; }
public:
    virtual void SetSimulate(bool bEnable) = 0;
    virtual void SetMass(float InMass) = 0;
    virtual void SetPos(const ::Vec3& InPos) = 0;
    virtual void SetRotation(const ::Vec4& InRotation) {}
    virtual void SetScale(const ::Vec3& InScale) = 0;
    virtual void SetGravity(bool bGravity) = 0;
    virtual void AddForce(const ::Vec3& InForce) = 0;
    virtual void SetVelocity(const ::Vec3& InVelocity) = 0;
    virtual void SetAngularVelocity(const ::Vec3& InVelocity) = 0;
public:
    virtual ::Vec3 GetPhysicsPos() = 0;
    virtual ::Vec4 GetPhysicsRotation() = 0;

public:
	std::shared_ptr<MPrimitiveComponent> GetPrimitiveComponent() { return Owner.lock(); }
protected:
	std::weak_ptr<MPrimitiveComponent> Owner;

public:
	std::shared_ptr<StaticMesh> GetMesh() { return MeshCache.lock(); }
protected:
    std::weak_ptr<StaticMesh> MeshCache;
};