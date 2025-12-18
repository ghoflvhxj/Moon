#pragma once

#include "SceneComponent.h"

class ENGINE_DLL MCameraComponent : public MSceneComponent
{
public:
	explicit MCameraComponent() = default;
	virtual ~MCameraComponent() = default;

public:
    virtual void BeginPlay() override;
    virtual void Update(const Time deltaTime) override;

protected:
    float ArmLength = 3.f;
    float TargetArmLength = 3.f;
    Vec3 TargetRot = {};

public:
    REFLECT(
        MCameraComponent
        , PROPERTY(TargetArmLength)
    )
};