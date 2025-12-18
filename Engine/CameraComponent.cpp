#include "CameraComponent.h"
#include "MoonEngine.h"

#include "World.h"
#include "Camera.h"
#include "DirectInput.h"
#include "DynamicMeshComponent.h"

#undef max

void MCameraComponent::BeginPlay()
{
    Super::BeginPlay();
}

void MCameraComponent::Update(const Time deltaTime)
{
    Super::Update(deltaTime);

    auto Lerp = [](float Current, float Target, float Speed) {
        return Current + ((Target - Current) * Speed);
    };

    if (MWorld* World = GetWorld())
    {
        if (World->IsPlaying())
        {
            if (auto& Camera = World->getMainCamera())
            {
                Vec3 PivotPos = getWorldTranslation();
                for (auto& [Name, Comp] : getOwningActor()->GetComponents())
                {
                    if (auto MeshComp = Comp->CastTo<DynamicMeshComponent>())
                    {
                        const Vec3& JointPos = MeshComp->GetJointPosition("bone001");
                        if (XMVector3Equal(XMLoadFloat3(&JointPos), XMLoadFloat3(&VEC3ZERO)) == false)
                        {
                            PivotPos = JointPos;
                        }
                    }
                }

                float mouseZ = static_cast<float>(InputManager::mouseMove(EAxis::Z));
                mouseZ = std::clamp(mouseZ, -1.f, 1.f) * 0.3f;
                TargetArmLength -= mouseZ;
                TargetArmLength = clamp(TargetArmLength, 1.f, 5.f);
                std::cout << mouseZ << std::endl;
                if (ArmLength != TargetArmLength)
                {
                    float NewArmLength = Lerp(ArmLength, TargetArmLength, deltaTime);
                    ArmLength = NewArmLength;
                }
                Vec3 Pos = {};
                XMStoreFloat3(&Pos, XMLoadFloat3(&PivotPos) - XMLoadFloat3(&GetForward()) * ArmLength);
                Camera->SetWorldTranslation(Pos);


                Vec3 CurrentRot = getRotation();
                float mouseX = static_cast<float>(InputManager::mouseMove(EAxis::X)) * 0.3f; // 1 = 디그리 3도 -> 라디안
                float mouseY = static_cast<float>(InputManager::mouseMove(EAxis::Y)) * 0.3f;
                mouseX = ToRadian(mouseX);
                mouseY = ToRadian(mouseY);
                TargetRot.x += mouseY;
                TargetRot.y += mouseX;
                TargetRot.x = clamp(TargetRot.x, ToRadian(-80.f), ToRadian(80.f));

                if (XMVector3Equal(XMLoadFloat3(&CurrentRot), XMLoadFloat3(&TargetRot)) == false)
                {
                    float RotX = Lerp(CurrentRot.x, TargetRot.x, std::max(PI, fabs(TargetRot.x - CurrentRot.x)) * deltaTime);
                    float RotY = Lerp(CurrentRot.y, TargetRot.y, std::max(PI, fabs(TargetRot.y - CurrentRot.y)) * deltaTime);
                    setRotation({ RotX, RotY, 0.f });
                    Camera->getComponent(ROOT_COMPONENT)->setRotation({ RotX, RotY, 0.f });
                }

                if (auto& Window = GetEngine()->GetWorldBoundedWindow(World))
                {
                    if (Window->IsForegorund())
                    {
                        Window->MouseCneter();
                    }
                }
            }
        }
    }


}
