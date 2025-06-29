#pragma once

#include "include.h"
#include "Core/Physics/Physics.h"

#include "Jolt/Jolt.h"
#include "Jolt/Core/Reference.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

class StaticMesh;

namespace JPH
{
    class TempAllocator;
    class JobSystem;
}

class ENGINE_DLL MJoltPhysics : public MPhysics
{
public:
    MJoltPhysics();
    virtual ~MJoltPhysics() = default;

public:
    virtual bool AddPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType, std::unique_ptr<MPhysicsObject>& OutPhysicsObject) override;

public:
    virtual void Update(float deltaTime) override;
    virtual void Release() override {}

public:
    JPH::TempAllocator* tempAllocator = nullptr;
    JPH::JobSystem* jobSystem = nullptr;
    JPH::JobSystem* jobSystemValidating = nullptr;
    JPH::PhysicsSystem physics_system;
};

class ENGINE_DLL MJoltPhysicsObject : public MPhysicsObject
{
public:
    MJoltPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType);

public:
    MJoltPhysics* Get()
    {
        return static_cast<MJoltPhysics*>(g_pPhysics.get());
    }
    JPH::PhysicsSystem* GetPhysicsSystem()
    {
        return &Get()->physics_system;
    }

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
    void AttachShape(JPH::Ref<JPH::Shape> InShape) { ShapeCache = InShape; }
    void SetBody(JPH::Body* InBody) { BodyCache = InBody; }
protected:
    JPH::Ref<JPH::Shape> ShapeCache;
    JPH::Body* BodyCache = nullptr;
};