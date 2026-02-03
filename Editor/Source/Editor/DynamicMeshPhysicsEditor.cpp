#include "DynamicMeshPhysicsEditor.h"

#include "MoonEngine.h"
#include "GraphicDevice.h"
#include "Module/Physics/CharacterPhysics.h"
#include "Renderer.h" // TODO Module/Rendering/Renderer.h

#include "Actor.h"
#include "DynamicMeshComponent.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "Framework/DynamicMeshActor/DynamicMeshActor.h"

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
}

//void MDynamicMeshPhysicsEditor::Test()
//{
//    MAssetEditor::Test();
//
//    std::vector<FBodyCapsuleData>& BodyCapsules = GetDynamicMeshPhysics()->GetCapsules();
//
//    HandleSkeleton(dynamicMesh->GetSkeleton().get(), [&](uint32 InJointIndex) {
//        if (ImGui::BeginPopupContextWindow("Test", ImGuiPopupFlags_MouseButtonRight))
//        {
//            if (ImGui::MenuItem("Add Capsule"))
//            {
//                GetDynamicMeshPhysics()->MakeCapsule(InJointIndex);
//            }
//            ImGui::EndPopup();
//        }
//    });
//}

void MDynamicMeshPhysicsEditor::Update()
{
    //std::vector<FBodyCapsuleData>& BodyCapsules = GetDynamicMeshPhysics()->GetCapsules();
    //auto DynamicMeshComp = Target->getComponent(ROOT_COMPONENT)->CastTo<DynamicMeshComponent>();


    //for (FBodyCapsuleData& BodyCapsule : BodyCapsules)
    {
        //const Mat4 JointMat = DynamicMeshComp->GetJointMatrix(BodyCapsule.AttachJointIndex);
        //Vec3 S, R, T;
        //DecomposeTransform(JointMat, S, R, T);

        //GetRenderer()->DrawCapsule(W.get(), BodyCapsule.GetRadius(), BodyCapsule.GetHalfHeight(), T, R);

        //const FJoint& AttachedJoint = dynamicMesh->GetJoint(BodyCapsule.AttachJointIndex);
        //Vec4 Quat = {};
        //XMStoreFloat4(&Quat, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&AttachedJoint.Rotation)));
    }
}

//void MDynamicMeshPhysicsEditor::SetAsset(std::shared_ptr<MAsset>& InAsset)
//{
//    MAssetEditor::SetAsset(InAsset);
//
//    if (auto DA = CreateActor<MDynamicMeshActor>(W))
//    {
//        DA->SetDynamicMesh(InAsset->GetAssetPath());
//        Target = DA;
//    }
//}
