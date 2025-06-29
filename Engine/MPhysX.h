#pragma once

#include "Core/Physics/Physics.h"

#include "NvidiaPhysX/PxPhysicsAPI.h"
#include "NvidiaPhysX/extensions/PxDefaultErrorCallback.h"
#include "NvidiaPhysX/extensions/PxDefaultAllocator.h"
using namespace physx;

class ENGINE_DLL MPhysX : public MPhysics
{
public:
	PxFoundation* Foundation = nullptr;
	PxPvd* VisualDebugger = nullptr;
    PxPvdTransport* Transport = nullptr;
	PxPhysics* Physics = nullptr;
	PxDefaultCpuDispatcher* CpuDispatcher = nullptr;
	PxScene* Scene = nullptr;
    PxCudaContextManager* CudaContextManager = nullptr;

public:
	MPhysX();
    virtual ~MPhysX();

public:
	PxPhysics* operator->();
    
public:
	virtual void Update(float deltaTime) override;
    virtual void Release() override;

protected:
	bool CreateConvex(const std::vector<Vec3>& Vertices, PxConvexMesh** ConvexMesh);

public:
    virtual bool AddPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType, std::unique_ptr<MPhysicsObject>& OutPhysicsObject) override;
};

class ENGINE_DLL MPhysXObject : public MPhysicsObject
{
public:
    MPhysXObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType);
    virtual ~MPhysXObject() = default;

public:
    MPhysX* Get()
    {
        return static_cast<MPhysX*>(g_pPhysics.get());
    }
    PxPhysics* GetPhysX()
    {
        return Get()->Physics;
    }
    PxScene* GetScene()
    {
        return Get()->Scene;
    }

public:
    virtual bool IsSimulating() override;
public:
    virtual void SetSimulate(bool bEnable) override;
    virtual void SetMass(float InMass) override;
    virtual void SetPos(const Vec3& InPos) override;
    virtual void SetScale(const Vec3& InScale) override;
    virtual void SetGravity(bool bInGravity) override;
    virtual void AddForce(const Vec3& InForce) override;
    virtual void SetVelocity(const Vec3& InVelocity) override;
    virtual void SetAngularVelocity(const Vec3& InVelocity) override;

public:
    virtual Vec3 GetPhysicsPos() override;
    virtual Vec4 GetPhysicsRotation() override;

public:
    void AttachShape(PxShape* InShape);
    bool IsValid() { return GetRigidActor() != nullptr; }

protected:
    physx::PxRigidActor* GetRigidActor();
protected:
    PxRigidStatic* RigidStatic = nullptr;
    PxRigidDynamic* RigidDynamic = nullptr;
    PxShape* Shape = nullptr;
};
