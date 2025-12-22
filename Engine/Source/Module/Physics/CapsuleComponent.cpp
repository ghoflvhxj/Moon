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

void MCollisionComponent::Update(const Time deltaTime)
{
    if (PhysicsObject)
    {
        setTranslation(PhysicsObject->GetPhysicsPos());
        SetRotation(PhysicsObject->GetPhysicsRotation());
    }

    Super::Update(deltaTime);
}

void MCollisionComponent::SetRotation(const Vec3& InRotation)
{
    if (PhysicsObject)
    {
        PhysicsObject->SetRotation(InRotation);
    }

    Super::SetRotation(InRotation);
}

void MCollisionComponent::AddRotation(const Vec3& InAdditiveRot)
{
    if (PhysicsObject)
    {
        const Vec3& Rot = PhysicsObject->GetPhysicsRotation();
        Vec3 NewRot = Rot;
        XMStoreFloat3(&NewRot, XMLoadFloat3(&Rot) + XMLoadFloat3(&InAdditiveRot));
        PhysicsObject->SetRotation(NewRot);
    }

    Super::AddRotation(InAdditiveRot);
}

void MCapsuleComponent::Update(const Time deltaTime)
{
    Super::Update(deltaTime);

    //getRenderer()->DrawCapsule(GetWorld(), CapsuleData.Radius, CapsuleData.HalfHeight, getWorldTranslation(), getRotation());
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

    Vec3 Movement = InMove;
    XMStoreFloat3(&Movement, XMVector3Rotate(XMLoadFloat3(&InMove), XMQuaternionRotationRollPitchYaw(0.f, getRotation().y, 0.f)));

    PhysicsObject->SetVelocity(Movement);
}