#pragma once

#include "SceneComponent.h"
#include "Mesh/Mesh.h" // FCapsuleData 참조용

class MPhysicsObject;

class ENGINE_DLL MCollisionComponent : public MSceneComponent
{
    REFLECT(
        MCollisionComponent
    )
};

class ENGINE_DLL MCapsuleComponent : public MCollisionComponent
{
public:
    virtual void BeginPlay() override;
    virtual void Update(const Time deltaTime) override;

public:
    void SetRadius(float InRadius);
    float GetRadius();

public:
    void SetHalfHeight(float InHalfHeight);
    float GetHalfHeight();

// MovementComponent를 만들면 옮겨야 하는 코드들
public:
    void AddMove(const Vec3& InMove);

protected:
    std::shared_ptr<MPhysicsObject> PhysicsObject;

protected:
    FCapsuleData CapsuleData;

    REFLECT(
        MCapsuleComponent
        , PROPERTY(CapsuleData)
    )
};