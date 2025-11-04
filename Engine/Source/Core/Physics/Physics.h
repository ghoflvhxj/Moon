#pragma once

#include "Include.h"

#include "Core/Asset.h"
#include "Module/Module.h"

#include "PhysicsEnum.h"
#include "Vertex.h"
#include "Mesh/Mesh.h" // FClothData 참조

class StaticMesh;
class MPhysicsObject;
class MPrimitiveComponent;
class MMeshComponent;
class DynamicMeshComponent;

struct FBodyCapsuleData;

struct FBodyConstructData
{
	std::shared_ptr<MPrimitiveComponent> PrimitiveComponent;
	std::shared_ptr<StaticMesh> Mesh;
	EPhysicsType PhysicsType;
    ::Vec3 Pos = VEC3ZERO;
    ::Vec4 Rot = VEC4ZERO;
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

public:
    // Component를 받도록 변경
    virtual void AddCloth(FBodyConstructData& InData, std::vector<FClothData>& ClothData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) {}
    virtual void AddCharacterBody(std::shared_ptr<DynamicMeshComponent> InDynamicMeshComp, FBodyCapsuleData& InBodyCapsuleData) {}

protected:
    std::vector<std::weak_ptr<MPhysicsObject>> SoftBodies;

    // 프리미티브ID - 피직스 오브젝트 쌍
    std::map<uint32, std::shared_ptr<MPhysicsObject>> PhysicsObjects;

protected:
    bool bSimulating = false;

    // 임시. 시뮬레이션 등록 컴포넌트
    std::vector<std::weak_ptr<MMeshComponent>> MeshComponents;

    REFLECT(MPhysicsEngine)
};

class ENGINE_DLL MPhysicsObject : std::enable_shared_from_this<MPhysicsObject>
{
public:
	MPhysicsObject(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent, std::shared_ptr<StaticMesh> InMesh)
		: Owner(InPrimitiveComponent)
		, MeshCache(InMesh)
    {

    }
    virtual ~MPhysicsObject() = default;

public:
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
    virtual void SetRotation(const ::Vec3& InRotation) {}
    virtual void SetScale(const ::Vec3& InScale) = 0;
    virtual void SetGravity(bool bGravity) = 0;
    virtual void AddForce(const ::Vec3& InForce) = 0;
    virtual void SetVelocity(const ::Vec3& InVelocity) = 0;
    virtual void SetAngularVelocity(const ::Vec3& InVelocity) = 0;
public:
    virtual ::Vec3 GetPhysicsPos() = 0;
    virtual ::Vec3 GetPhysicsRotation() = 0;

public:
	std::shared_ptr<MPrimitiveComponent> GetPrimitiveComponent() { return Owner.lock(); }
protected:
	std::weak_ptr<MPrimitiveComponent> Owner;

public:
	std::shared_ptr<StaticMesh> GetMesh() { return MeshCache.lock(); }
protected:
    std::weak_ptr<StaticMesh> MeshCache;
};