#pragma once

#include "include.h"
#include "Core/Physics/Physics.h"

#include "Jolt/Jolt.h"
#include "Jolt/Core/Reference.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Body/BodyID.h"

class StaticMesh;

namespace JPH
{
    class TempAllocator;
    class JobSystem;
}

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

class ENGINE_DLL MJoltPhysics : public MPhysics
{
public:
    MJoltPhysics();
    virtual ~MJoltPhysics() = default;

public:
    virtual bool AddPhysicsObject(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) override;
    virtual bool AddCloth(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) override;
    virtual void AddCloth(FPhysicsConstructData& InData, std::vector<FTest>& ClothData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject) override;

public:
    virtual void Update(float deltaTime) override;
    virtual void Release() override;

public:
    JPH::TempAllocator* tempAllocator = nullptr;
    JPH::JobSystem* jobSystem = nullptr;
    JPH::JobSystem* jobSystemValidating = nullptr;
    JPH::PhysicsSystem* physics_system = nullptr;
};

class ENGINE_DLL MJoltPhysicsObject : public MPhysicsObject
{
public:
    MJoltPhysicsObject(FPhysicsConstructData& InData);

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
    virtual void UpdateVertices(std::vector<::Vertex>& InVertices) override;
public:
    virtual bool IsSimulating() override;
public:
    virtual void SetSimulate(bool bEnable) override;
    virtual void SetMass(float InMass) override;
    virtual void SetPos(const ::Vec3& InPos) override;
    virtual void SetScale(const ::Vec3& InScale) override;
    virtual void SetGravity(bool bGravity) override;
    virtual void AddForce(const ::Vec3& InForce) override;
    virtual void SetVelocity(const ::Vec3& InVelocity) override;
    virtual void SetAngularVelocity(const ::Vec3& InVelocity) override;
public:
    virtual ::Vec3 GetPhysicsPos() override;
    virtual ::Vec4 GetPhysicsRotation() override;

public:
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


};