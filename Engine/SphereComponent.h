#pragma once
#ifndef __SPHERECOMPONENT_H__
#define __SPHERECOMPONENT_H__

#include "PrimitiveComponent.h"
#include "NvidiaPhysX/PxPhysicsAPI.h"

class ENGINE_DLL SphereComponent : public MPrimitiveComponent
{
public:
	explicit SphereComponent();

public:
	virtual void Update(const Time deltaTime) override;

	void AddForce(const Vec3& Force);

private:
	physx::PxGeometry*			PhysXGeometry;
	physx::PxRigidDynamic*		PhysXActor;
	physx::PxShape*				PhysXShape;
	physx::PxMaterial*			PhysXMaterial;
private:
	float Radius;
};

#endif