#include "StaticMeshActor.h"
#include "StaticMeshComponent.h"

#include "MoonEngine.h"
#include "World.h"

#include "DynamicMeshComponent.h"

using namespace DirectX;

MStaticMeshActor::MStaticMeshActor()
    : MActor()
{
    StaticMeshComp = std::make_shared<StaticMeshComponent>();
    AddComponent(ROOT_COMPONENT, StaticMeshComp);
}

void MStaticMeshActor::QuaternionToEuler_XYZ(Vec4 q, float& outPitch, float& outYaw, float& outRoll)
{
    float mag = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    float x = q.x / mag;
    float y = q.y / mag;
    float z = q.z / mag;
    float w = q.w / mag;

    // Pitch (X-axis)
    float t = 2.0f * (w * x - y * z);
    t = clampf(t, -1.0f, 1.0f);
    outPitch = asinf(t);

    // Yaw (Y-axis)
    outYaw = atan2f(2.0f * (w * y + z * x),
        1.0f - 2.0f * (x * x + y * y));

    // Roll (Z-axis)
    outRoll = atan2f(2.0f * (w * z + x * y),
        1.0f - 2.0f * (y * y + z * z));
}

void MStaticMeshActor::tick(const Time deltaTime)
{
    auto Iter = GetMainWorld()->GetActors().find("Player_0");
    if (Iter == GetMainWorld()->GetActors().end())
    {
        return;
    }

    auto& Comp = Iter->second->getComponent(ROOT_COMPONENT)->CastTo<DynamicMeshComponent>();
    if (Comp == nullptr)
    {
        return;
    }

    if (Comp->IsAnimPlaying() == false)
    {
        return;
    }

    std::string BoneName = "bone001";

    Vec3 Pos = Comp->GetJointPosition(BoneName);
    Pos.x /= 2.54f;
    Pos.y /= 2.54f;
    Pos.z /= 2.54f;
    SetWorldTranslation(Pos);

    Vec4 Quat = Comp->GetJointQuaternion(BoneName);
    Vec3 Rot = {};
    QuaternionToEuler_XYZ(Quat, Rot.x, Rot.y, Rot.z);
    getComponent(ROOT_COMPONENT)->setRotation(Rot);

    //std::cout << "Actor Pos: " << Pos << std::endl;

    //std::cout << "Mesh Angle: " << ToDegree(Rot.x) << ", " << ToDegree(Rot.y) << ", " << ToDegree(Rot.z) << std::endl;
}

void MStaticMeshActor::SetStaticMesh(const std::wstring& Path)
{
    StaticMeshComp->SetMesh(Path);
}

