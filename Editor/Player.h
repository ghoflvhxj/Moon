#pragma once

#include "Actor.h"

#include "Module/Physics/CapsuleComponent.h"
#include "DynamicMeshComponent.h"
#include "CameraComponent.h"

struct MAnimation;

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

    float HealthPoint = 100.f;
    bool bTest = false;

    std::vector<std::shared_ptr<MAnimation>> Anims;

    std::shared_ptr<MAnimation> IdleAnim;
    std::shared_ptr<MAnimation> WalkAnim;
    std::shared_ptr<MAnimation> WalkToIdleAnim;

    Vec3 PrevBonePos = VEC3ZERO;
    bool bResetBonePose = true;

    REFLECT(
        Player
        , PROPERTY(CharacterMeshComponent)
        , PROPERTY(CapsuleComponent)
        , PROPERTY(CameraComponent)
        , PROPERTY(IdleAnim)
        , PROPERTY(WalkAnim)
        , PROPERTY(WalkToIdleAnim)
    )
};