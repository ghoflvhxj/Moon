#pragma once
#ifndef __PHYSX_H__
#define __PHYSX_H__

#include "NvidiaPhysX/PxPhysicsAPI.h"
#include "GraphicDevice.h"

using namespace physx;

class ENGINE_DLL PhysXX
{
public:
	static PxDefaultErrorCallback g_DefaultErrorCallback;
	static PxDefaultAllocator g_DefaultAllocator;

	PxFoundation* Foundation;
	PxPvd* VisualDebugger;
	PxPhysics* Physics;
	PxDefaultCpuDispatcher* CpuDispatcher;
	PxScene* Scene;
	PxMaterial* Material;
	PxCooking* Cooking;

	PxRigidStatic* Plane;

public:
	PhysXX();

	PxPhysics* operator->();

	void Update(float deltaTime);

	bool CreateConvex(const std::vector<Vec3>& Vertices, PxConvexMesh** ConvexMesh);
};


#endif