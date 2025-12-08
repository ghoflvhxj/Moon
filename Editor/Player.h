#pragma once

#include "Actor.h"

#include "Module/Physics/CapsuleComponent.h"
#include "DynamicMeshComponent.h"
#include "CameraComponent.h"

class Player : public MActor
{
public:
    explicit Player();
    virtual ~Player();

public:
    virtual void BeginPlay() override;

protected:
    virtual void tick(const Time deltaTime) override;

public:
    void JsonSaveTest(bool bPretty = false);
    void JsonLoadTest();

private:
    std::shared_ptr<DynamicMeshComponent> CharacterMeshComponent;
    std::shared_ptr<MCapsuleComponent> CapsuleComponent;
    std::shared_ptr<MCameraComponent> CameraComponent;

    REFLECT(
        Player
        , PROPERTY(CharacterMeshComponent)
        , PROPERTY(CapsuleComponent)
        , PROPERTY(CameraComponent)
    )
};