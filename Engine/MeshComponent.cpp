#include "MeshComponent.h"

#include "Renderer.h"
#include "Core/Physics/Physics.h"
#include "Core/ResourceManager.h"

void MMeshComponent::AddForce(const Vec3& InForce)
{
    if (PhysicsObject)
    {
        PhysicsObject->AddForce(InForce);
    }
}

void MMeshComponent::Clothing()
{
    
}

void MMeshComponent::RemovePhysics()
{
    if (PhysicsObject)
    {
        PhysicsObject->Remove();
        PhysicsObject.reset();
    }
}

void MMeshComponent::SetPhysics(bool bInPhysics, bool bForce)
{
    if (bPhysics == bInPhysics && bForce == false)
    {
        return;
    }

    bPhysics = bInPhysics;
}

void MMeshComponent::SetPhysicsSimulate(bool bInSimulate, bool bForce /*= false*/)
{
    if (PhysicsObject == nullptr)
    {
        return;
    }

    if (bInSimulate != PhysicsObject->IsSimulating())
    {
        bPhysicsSimulate = bInSimulate;
        PhysicsObject->SetSimulate(bPhysicsSimulate);
    }
}