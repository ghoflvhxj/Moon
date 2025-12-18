#include "CapsuleBody.h"

#include "MoonEngine.h"
#include "Renderer.h"

#include "DynamicMeshComponent.h"

using namespace DirectX;

void MCapsuleBody::Update(float DeltaTime)
{
    auto& DynamicMeshComp = GetDynamicMeshComponent();
    if (DynamicMeshComp == nullptr)
    {
        return;
    }

    uint32 JointIndex = BodyCapsuleData.AttachJointIndex;
    const FJoint& Joint = DynamicMeshComp->GetJoint(JointIndex);

    const ::Vec3& DXJointPos = DynamicMeshComp->GetJointPosition(JointIndex, BodyCapsuleData.TranslationOffset);
    const JPH::Vec3& JoltJointPos = MJoltPhysics::ToJPHPos(DXJointPos);

    const ::Vec4& DXJointQuat = DynamicMeshComp->GetJointQuaternion(JointIndex);
    const JPH::Quat& JoltJointQuat = MJoltPhysics::DXQuatToJPHQuat(DXJointQuat);

    GetPhysicsSystem()->GetBodyInterface().MoveKinematic(GetBodyID(), JoltJointPos, JoltJointQuat, DeltaTime);
    
    //std::cout << "bone019 Jolt Pos: " << JoltJointPos.GetX() << ", " << JoltJointPos.GetY() << ", " << JoltJointPos.GetZ() << std::endl;

    //getRenderer()->DrawCoordinate(GetMainWorld().get(), DynamicMeshComp->GetJointPosition("bone019"), DynamicMeshComp->GetJointQuaternion("bone019"));
    //getRenderer()->DrawCapsule(GetMainWorld().get(), BodyCapsuleData.CapsuleData.Radius, BodyCapsuleData.CapsuleData.HalfHeight, DynamicMeshComp->GetJointPosition(JointIndex), DynamicMeshComp->GetJointQuaternion(JointIndex));
}

void MCapsuleBody::Render()
{
    auto& Renderer = getRenderer();

    JPH::Vec3 JoltBodyPos = GetBody().GetPosition();
    ::Vec3 DXBodyPos = { JoltBodyPos.GetX(), JoltBodyPos.GetY(), -JoltBodyPos.GetZ() };

    JPH::Quat JoltBodyRot = GetBody().GetRotation();
    ::Vec4 DxBodyQuat = MJoltPhysics::JoltQuatToDXQuat(JoltBodyRot);

    if (BodyCapsuleData.AttachJointIndex != -1)
    {
        //Renderer->DrawCoordinate(GetMainWorld().get(), DXBodyPos, DxBodyQuat);
        Renderer->DrawCapsule(GetDynamicMeshComponent()->GetWorld(), BodyCapsuleData.CapsuleData.Radius, BodyCapsuleData.CapsuleData.HalfHeight, DXBodyPos, DxBodyQuat);
    }

}

std::shared_ptr<DynamicMeshComponent> MCapsuleBody::GetDynamicMeshComponent()
{
    if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = GetPrimitiveComponent())
    {
        return PrimitiveComp->CastToShared<DynamicMeshComponent>();
    }

    return nullptr;
}

std::shared_ptr<DynamicMesh> MCapsuleBody::GetDynamicMesh()
{
    if (std::shared_ptr<MMesh>& Mesh = MeshCache.lock())
    {
        return Mesh->CastToShared<DynamicMesh>();
    }

    return nullptr;
}
