#include "DynamicMeshPhysicsEditor.h"

#include "MoonEngine.h"
#include "GraphicDevice.h"
#include "Module/Physics/CharacterPhysics.h"

#include "Renderer.h" // TODO Module/Rendering/Renderer.h

#include "Mesh/DynamicMesh/DynamicMesh.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

// 임시
#include "Launch.h"

using namespace DirectX;
using namespace ImGui;

MDynamicMeshPhysicsEditor::MDynamicMeshPhysicsEditor(MObject* InObject)
    : MAssetEditor(InObject)
{
    dynamicMesh = static_cast<DynamicMesh*>(InObject);
}

MDynamicMeshPhysicsEditor::~MDynamicMeshPhysicsEditor()
{
    ImGui::DestroyContext(Context);
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

    ImGuiContext* PrevContext = ImGui::GetCurrentContext();

    ImGui::SetCurrentContext(Context);

    ImGui::SetCurrentContext(PrevContext);
}
