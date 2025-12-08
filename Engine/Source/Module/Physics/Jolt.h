#pragma once

#include "include.h"
#include "Physics.h"

#include "Jolt/Jolt.h"
#include "Jolt/Core/Reference.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyID.h"

#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"

#include "Jolt/Physics/Character/Character.h"

#include "Mesh/Mesh.h"

class MMeshComponent;
class StaticMesh;

namespace JPH
{
    class TempAllocator;
    class JobSystem;
}

// 이름은 일단 임시로
struct FBodyCapsuleDataWrapper
{

};

struct FVertexKey
{
    bool operator==(const FVertexKey& Rhs) const
    {
        auto IsEqual = [](float lhs, float rhs)->bool {
            return std::fabsf(lhs - rhs) < 0.00001f;
        };
        return IsEqual(x, Rhs.x) && IsEqual(y, Rhs.y) && IsEqual(z, Rhs.z);
    }

    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
};

namespace std
{
    template <>
    struct hash<FVertexKey>
    {
        size_t operator()(const FVertexKey& VertexKey) const
        {
            constexpr float Precision = 10000.f;
            size_t h0 = std::hash<float>{}(std::round(VertexKey.x * Precision));
            size_t h1 = std::hash<float>{}(std::round(VertexKey.x * Precision));
            size_t h2 = std::hash<float>{}(std::round(VertexKey.x * Precision));

            size_t h = h0;
            h ^= h1 + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= h2 + 0x9e3779b9 + (h << 6) + (h >> 2);

            return h;
        }
    };
};

class ENGINE_DLL MJoltPhysics : public MPhysicsEngine
{
public:
    MJoltPhysics() = default;
    virtual ~MJoltPhysics() = default;

public:
    virtual bool Initialize() override;
    virtual void Update() override;
    virtual void Render() override;
    virtual void Release() override;
    virtual void StartSimulate(MWorld* InWorld) override;

public:
    static JPH::Quat DXQuatToJPHQuat(const ::Vec4& InQuat);
    static JPH::Quat DXAngleToJPHQuat(const ::Vec3& InRot);
    static ::Vec4 JoltQuatToDXQuat(JPH::Quat InQuat);
    static JPH::Vec3 ToJPHPos(const ::Vec3& InPos);
    static JPH::Vec3 ToJPHPos(const ::Vec4& InPos);
public:
    // 메시의 ConvexHull을 만들어 저장함
    virtual void LoadTest() override;
    virtual void SaveTest(std::shared_ptr<MMesh> Mesh) override;

    JPH::ConvexHullShapeSettings MakeConvexHull(FBodyConstructData& InData);
    JPH::MeshShapeSettings MakeMeshShape(std::shared_ptr<StaticMesh> InMesh);
    JPH::CapsuleShapeSettings MakeCapsule(float InHalfHeight, float InRadius);
    JPH::SphereShapeSettings MakeSphere(float InRadius);
    //virtual void MakeKinematic(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject);

public:
    JPH::EMotionType ConvertPhysicsType(EPhysicsType InType);
public:
    virtual void AddCloth(FBodyConstructData& InData, std::vector<FClothData>& ClothData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) override;
    virtual void AddCharacterPhyscics(std::shared_ptr<DynamicMeshComponent> InDynamicMeshComp, FBodyCapsuleData& InBodyCapsuleData) override;
    virtual void AddCharacterCollision(std::shared_ptr<MCollisionComponent> InComp, const FCapsuleData& InCapsuleData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) override;
    JPH::Character* NewCharacter = nullptr;
protected:
    std::vector<std::shared_ptr<class MCapsuleBody>> CapsuleBodies;

public:
    virtual void Constraint(std::shared_ptr<MPhysicsObject>& Lhs, std::shared_ptr<MPhysicsObject>& Rhs);

protected:
    // Shape를 이용해 바디를 만드는 함수
    void CreateBody(JPH::RefConst<JPH::Shape> InShape, const FBodyConstructData& InData, std::shared_ptr<class MBodyObject> InBodyObject);

public:
    JPH::TempAllocator* tempAllocator = nullptr;
    JPH::JobSystem* jobSystem = nullptr;
    JPH::JobSystem* jobSystemValidating = nullptr;
    JPH::PhysicsSystem* physics_system = nullptr;

    JPH::Vec3 TempRot = {};

    REFLECT(MJoltPhysics)
};

class ENGINE_DLL MBodyObject : public MPhysicsObject
{
public:
    MBodyObject(const FBodyConstructData& InData);

public:
    MJoltPhysics* Get()
    {
        return static_cast<MJoltPhysics*>(g_pPhysics.get());
    }
    JPH::PhysicsSystem* GetPhysicsSystem()
    {
        return Get()->physics_system;
    }

public:
    virtual void MoveTo(const ::Vec3& TargetPos) override;
    virtual void Remove() override;
public:
    virtual bool IsSimulating() override;
public:
    virtual void SetSimulate(bool bEnable) override;
    virtual void SetMass(float InMass) override;
    virtual void SetPos(const ::Vec3& InPos) override;
    virtual void SetRotation(const ::Vec4& InRotation) override;
    //virtual void SetRotation(const ::Vec3& InRotation) override;
    virtual void SetScale(const ::Vec3& InScale) override;
    virtual void SetGravity(bool bGravity) override;
    virtual void AddForce(const ::Vec3& InForce) override;
    virtual void SetVelocity(const ::Vec3& InVelocity) override;
    virtual void SetAngularVelocity(const ::Vec3& InVelocity) override;
public:
    virtual ::Vec3 GetPhysicsPos() override;
    virtual ::Vec3 GetPhysicsRotation() override;

public:
    JPH::Body& GetBody();
    void SetBodyID(const JPH::BodyID InBodyID) { BodyIDCache = InBodyID; }
    JPH::BodyID GetBodyID() { return BodyIDCache; }
protected:
    JPH::BodyID BodyIDCache;

public:
    void SetMeshIndices(std::vector<uint32> InMeshIndices) { MeshIndices = InMeshIndices; }
    const std::vector<uint32>& GetMeshIndices() const { return MeshIndices; }
protected:
    std::vector<uint32> MeshIndices;

public:
    void SetVertexIndices(std::unordered_map<FVertexKey, uint32>& Rhs) { VertexIndex = std::move(Rhs); }
    uint32 GetVertexIndex(const ::Vec3& Pos);
protected:
    // 소프트 바디 버텍스, 인덱스 쌍
    std::unordered_map<FVertexKey, uint32> VertexIndex;

    // 임시
public:
    JPH::Vec3 CachePos = JPH::Vec3::sZero();
    JPH::Vec3 PrevVeloc = JPH::Vec3::sZero();

public:
    void SkinVertices(bool bHard, JPH::TempAllocator* Alloc);

    bool bTest = false;

    bool bOrigin = false;
    JPH::Vec3 Origin = JPH::Vec3::sZero();
};