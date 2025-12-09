#include "CapsuleComponent.h"

#include "MoonEngine.h"
#include "Module/Physics/Physics.h"
#include "Renderer.h"

void MCapsuleComponent::BeginPlay()
{
    Super::BeginPlay();

    GetPostLoopDelegate().Add(this, [this]() {
        if (auto& Physics = GetPhysics())
        {
            Physics->AddCharacterCollision(GetShared(), CapsuleData, PhysicsObject);
        }
    });
}

void MCapsuleComponent::Update(const Time deltaTime)
{
    Super::Update(deltaTime);

    Vec3 Pos = getWorldTranslation();
    if (PhysicsObject)
    {
        setTranslation(PhysicsObject->GetPhysicsPos());
        setRotation(PhysicsObject->GetPhysicsRotation());
        Pos = PhysicsObject->GetPhysicsPos();
    }

    getRenderer()->DrawCapsule(GetWorld(), CapsuleData.Radius, CapsuleData.HalfHeight, Pos, VEC3ZERO);
}

void MCapsuleComponent::SetRadius(float InRadius)
{
    CapsuleData.Radius = InRadius;
}

void MCapsuleComponent::SetHalfHeight(float InHalfHeight)
{
    CapsuleData.HalfHeight = InHalfHeight;
}

void MCapsuleComponent::AddMove(const Vec3& InMove)
{
    if (PhysicsObject == nullptr)
    {
        return;
    }

    PhysicsObject->SetVelocity(InMove);
}
