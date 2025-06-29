#pragma once

#include "Include.h"
#include "PhysicsEnum.h"

class StaticMesh;

class ENGINE_DLL MPhysicsObject
{
public:
    MPhysicsObject(std::shared_ptr<StaticMesh> InMesh)
        : MeshCache(InMesh)
    {

    }
    virtual ~MPhysicsObject() = default;

public:
    virtual bool IsSimulating() = 0;
public:
    virtual void SetSimulate(bool bEnable) = 0;
    virtual void SetMass(float InMass) = 0;
    virtual void SetPos(const ::Vec3& InPos) = 0;
    virtual void SetScale(const ::Vec3& InScale) = 0;
    virtual void SetGravity(bool bGravity) = 0;
    virtual void AddForce(const ::Vec3& InForce) = 0;
    virtual void SetVelocity(const ::Vec3& InVelocity) = 0;
    virtual void SetAngularVelocity(const ::Vec3& InVelocity) = 0;
public:
    virtual ::Vec3 GetPhysicsPos() = 0;
    virtual ::Vec4 GetPhysicsRotation() = 0;

protected:
    std::weak_ptr<StaticMesh> MeshCache;
};

class ENGINE_DLL MPhysics
{
public:
    MPhysics() = default;
    ~MPhysics() = default;

public:
    virtual void Update(float deltaTime) = 0;
    virtual void Release() = 0;

public:
    virtual bool AddPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType, std::unique_ptr<MPhysicsObject>& OutPhysicsObject) = 0;
};