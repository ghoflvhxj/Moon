#include "CapsuleBody.h"

#include "MoonEngine.h"
#include "Renderer.h"

#include "DynamicMeshComponent.h"

using namespace DirectX;

void MCapsuleBody::Update(float DeltaTime)
{
    static float TotalTime = 0.f;
    TotalTime += DeltaTime;

    JPH::Vec3 JoltPos = {};
    JPH::Quat JoltQuat = JPH::Quat::sIdentity();

    auto& DynamicMeshComp = GetDynamicMeshComponent();
    if (DynamicMeshComp == nullptr)
    {
        return;
    }

    //uint32 JointIndex = CapsuleData.AttachJointIndex;
    uint32 JointIndex = DynamicMeshComp->GetDynamicMesh()->GetJointIndex("bone019");

    Mat4 Matrix = {};
    XMStoreFloat4x4(&Matrix, XMMatrixTranslationFromVector(XMLoadFloat3(&BodyCapsuleData.TranslationOffset)) * XMLoadFloat4x4(&DynamicMeshComp->GetJointMatrix(JointIndex)));
    Vec3 XMPos = GetPos(Matrix);
    XMPos.x /= 2.54f;
    XMPos.y /= 2.54f;
    XMPos.z /= 2.54f;
    JoltPos = MJoltPhysics::ToJPHPos(XMPos);

    ::Vec4 JointQuat = { 0.f, 0.f, 0.f, 1.f };
    JointQuat = DynamicMeshComp->GetJointQuaternion(JointIndex);
    JoltQuat = MJoltPhysics::DXQuatToJPHQuat(JointQuat);

    JPH::Vec3 tt = JoltQuat.GetEulerAngles();

    //std::cout << "XM Pos: " << JointPos << std::endl;
    //std::cout << "XM Angle: " << ToDegree(JointAngle.x) << ", " << ToDegree(JointAngle.y) << ", " << ToDegree(JointAngle.z) << std::endl;
    //std::cout << "Jolt Angle: " << ToDegree(tt.GetX()) << ", " << ToDegree(tt.GetY()) << ", " << ToDegree(tt.GetZ()) << std::endl;

    GetPhysicsSystem()->GetBodyInterface().MoveKinematic(GetBodyID(), JoltPos, JoltQuat, DeltaTime);
}

void MCapsuleBody::Render()
{
    auto& Renderer = getRenderer();

    JPH::Vec3 JoltBodyPos = GetBody().GetPosition();
    ::Vec3 BodyPos = { JoltBodyPos.GetX(), JoltBodyPos.GetY(), -JoltBodyPos.GetZ() };
    //::Vec3 BodyPos = GetPos(DynamicMeshComp->GetJointMatrix(JointIndex));

    ::Vec4 DxBodyQuat = {};
    JPH::Vec3 JoltBodyRot = GetBody().GetRotation().GetEulerAngles();
    XMStoreFloat4(&DxBodyQuat, XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(-JoltBodyRot.GetX(), -JoltBodyRot.GetY(), JoltBodyRot.GetZ())));

    if (BodyCapsuleData.AttachJointIndex != -1)
    {
        const Vec3& JointScale = VEC3ONE;
        if (BodyCapsuleData.PrimitiveID == -1)
        {
            //BodyCapsuleData.PrimitiveID = Renderer->DrawCapsule(BodyCapsuleData.CapsuleData.Radius, BodyCapsuleData.CapsuleData.HalfHeight);
        }

        //Renderer->UpdatePrimitiveTransform(BodyCapsuleData.PrimitiveID, BodyPos, DxBodyQuat, JointScale);
    }
}

std::shared_ptr<DynamicMeshComponent> MCapsuleBody::GetDynamicMeshComponent()
{
    if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = GetPrimitiveComponent())
    {
        return PrimitiveComp->CastTo<DynamicMeshComponent>();
    }

    return nullptr;
}

std::shared_ptr<DynamicMesh> MCapsuleBody::GetDynamicMesh()
{
    if (std::shared_ptr<MMesh>& Mesh = MeshCache.lock())
    {
        return Mesh->CastTo<DynamicMesh>();
    }

    return nullptr;
}
