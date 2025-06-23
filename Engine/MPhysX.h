#pragma once

#include "NvidiaPhysX/PxPhysicsAPI.h"
#include "NvidiaPhysX/extensions/PxDefaultErrorCallback.h"
#include "NvidiaPhysX/extensions/PxDefaultAllocator.h"
using namespace physx;

class ENGINE_DLL PhysXX
{
public:
	static PxDefaultErrorCallback g_DefaultErrorCallback;
	static PxDefaultAllocator g_DefaultAllocator;

	PxFoundation* Foundation = nullptr;

	PxPvd* VisualDebugger = nullptr;
    PxPvdTransport* Transport = nullptr;

	PxPhysics* Physics = nullptr;
	PxDefaultCpuDispatcher* CpuDispatcher = nullptr;
	PxScene* Scene = nullptr;
	PxMaterial* Material = nullptr;
	//PxCooking* Cooking = nullptr;

	PxRigidStatic* Plane = nullptr;

public:
	PhysXX();
    virtual ~PhysXX();

    void Release();

	PxPhysics* operator->();

	void Update(float deltaTime);

	bool CreateConvex(const std::vector<Vec3>& Vertices, PxConvexMesh** ConvexMesh);
};
