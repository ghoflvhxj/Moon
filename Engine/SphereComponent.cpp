#include "stdafx.h"
#include "SphereComponent.h"

#include "MainGame.h"

#include <PxPhysicsAPI.h>
#include "MPhysX.h"

using namespace physx;

SphereComponent::SphereComponent()
	: PhysXGeometry{ nullptr }
	, PhysXActor{ nullptr }
	, PhysXShape{ nullptr }
	, Radius{ 1.f }
{
	if (g_pPhysics == nullptr)
	{
		return;
	}

	if (g_pPhysics->Physics == nullptr)
	{
		return;
	}

	PhysXMaterial = (*g_pPhysics)->createMaterial(0.f, 0.f, 0.0f);
	PhysXGeometry = &PxSphereGeometry(Radius);
	PhysXActor = (*g_pPhysics)->createRigidDynamic(PxTransform(0.f, 3.f, 0.f));
	PhysXShape = (*g_pPhysics)->createShape(*PhysXGeometry, *PhysXMaterial);
	PhysXActor->attachShape(*PhysXShape);
	g_pPhysics->Scene->addActor(*PhysXActor);
}

void SphereComponent::Update(const Time deltaTime)
{
	if (PhysXActor)
	{
		PxTransform PhysXTransform = PhysXActor->getGlobalPose();
		setTranslation(PhysXTransform.p.x, PhysXTransform.p.y, PhysXTransform.p.z);

		PxVec3 Rotation(PxIdentity);
		PhysXTransform.rotate(Rotation);
		setRotation(Vec3(Rotation.x, Rotation.y, Rotation.z));
	}

	PrimitiveComponent::Update(deltaTime);
}

void SphereComponent::AddForce(const Vec3& Force)
{
	PhysXActor->addForce(PxVec3(Force.x, Force.y, Force.z));
}
