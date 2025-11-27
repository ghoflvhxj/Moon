#pragma once

#include "Actor.h"

#include "Module/Physics/CapsuleComponent.h"
#include "DynamicMeshComponent.h"

class MMeshComponent;
class StaticMeshComponent;
class DynamicMeshComponent;
class TerrainComponent;
class MTexture;
class MPointLightComponent;
class MDirectionalLightComponent;
class SkyComponent;
class MCapsuleComponent;

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

    REFLECT(
        Player
        , PROPERTY(CharacterMeshComponent)
        , PROPERTY(CapsuleComponent)
    )
};