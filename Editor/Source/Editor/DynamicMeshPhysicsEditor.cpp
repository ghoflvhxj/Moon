#include "DynamicMeshPhysicsEditor.h"

#include "Module/Physics/CharacterPhysics.h"

#include "Renderer.h" // TODO Module/Rendering/Renderer.h

#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "imgui.h"

using namespace DirectX;

MDynamicMeshPhysicsEditor::MDynamicMeshPhysicsEditor(MObject* InObject)
    : MAssetEditor(InObject)
{
    dynamicMesh = static_cast<DynamicMesh*>(InObject);
}

void MDynamicMeshPhysicsEditor::Test()
{
    std::vector<FBodyCapsuleData>& BodyCapsules = GetDynamicMeshPhysics()->GetCapsules();

    HandleSkeleton(dynamicMesh->GetSkeleton().get(), [&](uint32 InJointIndex) {
        if (ImGui::BeginPopupContextWindow("Test", ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Add Capsule"))
            {
                GetDynamicMeshPhysics()->MakeCapsule(InJointIndex);
            }
            ImGui::EndPopup();
        }
    });
}

void MDynamicMeshPhysicsEditor::Render()
{
    std::vector<FBodyCapsuleData>& BodyCapsules = GetDynamicMeshPhysics()->GetCapsules();

    for (FBodyCapsuleData& BodyCapsule : BodyCapsules)
    {
        GetRenderer()->DrawCapsule(BodyCapsule.GetRadius(), BodyCapsule.GetHalfHeight());

        const FJoint& AttachedJoint = dynamicMesh->GetJoint(BodyCapsule.AttachJointIndex);

        Vec4 Quat = {};
        XMStoreFloat4(&Quat, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&AttachedJoint.Rotation)));

        GetRenderer()->UpdatePrimitiveTransform(0, AttachedJoint.Position, Quat, VEC3ONE);
    }
}
