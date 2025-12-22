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

    auto GetSign = [](float InValue) {
        return InValue >= 0.f ? 1 : -1;
    };

    if (MWorld* World = GetWorld())
    {
        if (World->IsPlaying())
        {
            // 카메라 컴포넌트 각도 설정
            //float mouseX = clamp(static_cast<float>(InputManager::mouseMove(EAxis::X)), -5.f, 5.f);
            //float mouseY = clamp(static_cast<float>(InputManager::mouseMove(EAxis::Y)), -5.f, 5.f);
            float mouseX = static_cast<float>(InputManager::mouseMove(EAxis::X)) * 0.3f;
            float mouseY = static_cast<float>(InputManager::mouseMove(EAxis::Y)) * 0.3f;

            if (mouseX != 0.f)
            {
                if (GetSign(CachedX) != GetSign(mouseX))
                {
                    TargetRot.y = GetWorldRotation().y;
                    if (TargetRot.y < 0.f)  // -PI ~ PI
                    {
                        TargetRot.y += PI2; // 0 ~ 2PI, ex) -10 -> 350, -100 -> 260
                    }
                }

                CachedX = mouseX;
            }

            mouseX = ToRadian(mouseX);
            mouseY = ToRadian(mouseY);
            TargetRot.x += mouseY;
            TargetRot.x = clamp(TargetRot.x, ToRadian(-80.f), ToRadian(80.f));
            TargetRot.y += mouseX;

            /***********
            T:0,    R: 0

            T:90,   R: 90
            T:180,  R: 180
            T:270:  R: -90  이거는 T - 2PI;

            T:-90,  R: -90
            T:-180, R: -180
            T:-270, R: 90   이거는 T + 2PI
            ***********/ 
            TargetRot.y = fmod(TargetRot.y, PI2); 
            if (TargetRot.y > PI)
            {
                TargetRot.y -= PI2;
            }
            else if (TargetRot.y < -PI)
            {
                TargetRot.y += PI2;
            }

            Vec3 CurrentRot = GetWorldRotation(); // -PI ~ PI

            /***********
            Cur: -1,    Tar: 1,     RD: 2,      LD: -358
            Cur: 1,     Tar: -1,    RD: 358,    LD: -2

            Cur: 80,    Tar: 90,    RD: 10,     LD: -350
            Cur: 90,    Tar: 80,    RD: 350,    LD: -10

            
            Cur < Tar -> RD = T - C, LD = RD - 2PI
            Tar < Cur -> RD = T - C + 2PI, LD = RD - 2PI
            방향은 오직 마우스 입력! 으로만 결정됨
            방향이 어디인지 Delta를 구분해서 LeftDelta, RightDelta로 구분해야 하고
            공식은 각도 차이에 따라 다름
            ***********/
            if (XMVector3Equal(XMLoadFloat3(&CurrentRot), XMLoadFloat3(&TargetRot)) == false)
            {
                float DirY = GetSign(CachedX) > 0.f ? 1.f : -1.f;

                float DeltaY = 0.f;
                if (CurrentRot.y < TargetRot.y)
                {
                    DeltaY = TargetRot.y - CurrentRot.y;
                }
                else if(CurrentRot.y > TargetRot.y)
                {
                    DeltaY = TargetRot.y - CurrentRot.y + PI2;
                }

                if (bool IsLeft = DirY < 0.f)
                {
                    DeltaY -= PI2;
                }

                float Scale = fabs(DeltaY);

                AddRotation({ (TargetRot.x - CurrentRot.x) * deltaTime, DirY * Scale * std::max(PI, Scale) * deltaTime , 0.f });
            }

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

                // 카메라 위치 설정
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

                if (auto& Window = GetEngine()->GetWorldBoundedWindow(World))
                {
                    if (Window->IsForegorund())
                    {
                        Window->MouseCneter();
                    }
                }

                Camera->getComponent(ROOT_COMPONENT)->SetRotation(GetWorldRotation());

                //std::cout << "MouseMove: " << mouseX << ", " << mouseY << std::endl;
                //std::cout << "TargetRot: " << TargetRot << std::endl;
            }
        }
    }


}
