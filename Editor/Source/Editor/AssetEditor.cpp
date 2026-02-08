#include "AssetEditor.h"
#include "MoonEngine.h"

#include "DirectInput.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

#include "Core/Asset.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"

#include "Module/Physics/Jolt.h"
#include "Module/Physics/CharacterPhysics.h"
#include "Renderer.h"

#include "World.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"
#include "Framework/DynamicMeshActor/DynamicMeshActor.h"
#include "Framework/DirectionalLightActor/DirectionalLightActor.h"
#include "Framework/StaticMeshActor/StaticMeshActor.h"
#include "Camera.h"

#include "MainWindow.h"
#include "WindowManager.h"

#include "Core/FileSystem.h"

#include <commdlg.h>

using namespace DirectX;

MAssetEditor::MAssetEditor()
    //: AssetOwningObject(InObject)
{
    WeakJolt = GetEngine()->GetModule<MJoltPhysics>();
}

void MAssetEditor::HandleObject()
{
    if (SourceObject->IsA<MAsset>() == false)
    {
        return;
    }

    // 원본
    Asset = SourceObject->CastToShared<MAsset>();
    assert(Asset);

    Title = Asset->GetTypeDesc()->Name + " Editor" + "(" + WStringToString(Asset->GetAssetPath()) + ")";

    // 애셋은 하나만 존재하도록 시스템화 되어있으니, 수동으로 복사본을 만들어야 함
    WorkingAsset = WorkingObject->CastToShared<MAsset>();
    assert(WorkingAsset);
    WorkingAsset->Load(Asset->GetAssetPath());

    W->getMainCamera()->SetWorldTranslation({ 0.f, 0.f, -2.f });
    W->getMainCamera()->setLookMode(MCamera::LookMode::At);

    if (WorkingAsset->IsA<DynamicMesh>())
    {
        if (auto DA = CreateActor<MDynamicMeshActor>(W))
        {
            DA->SetDynamicMesh(WorkingAsset->GetAssetPath());
            Target = DA;
        }
    }
    else if(WorkingAsset->IsA<StaticMesh>())
    {
        if (auto SA = CreateActor<MStaticMeshActor>(W))
        {
            SA->SetStaticMesh(WorkingAsset->GetAssetPath());
            SA->SetWorldTranslation({ 0.f, 0.f, 10.f });
            Target = SA;
        }
    }
    else if (WorkingAsset->IsA<MMaterial>())
    {
        if (auto SA = CreateActor<MStaticMeshActor>(W))
        {
            SA->SetStaticMesh(TEXT("Base/Sphere.json"));
            SA->GetStaticMeshCompoent()->SetMaterial(0, WorkingAsset->CastToShared<MMaterial>());
            Target = SA;
        }
    }
}

const std::wstring& MAssetEditor::GetPath() const
{
    return Asset->GetAssetPath();
}

void MAssetEditor::Update()
{
    if (Target == nullptr)
    {
        return;
    }

    if (W->IsMouseInViewport() && W->IsForegorund())
    {
        if (auto RootComponent = Target->getComponent(ROOT_COMPONENT))
        {
            if (InputManager::mousePress(MOUSEBUTTON::RB))
            {
                Vec2 CurrentMouesPos = T->GetMousePos();

                if (bControl)
                {
                    Vec3 DeltaMousePos = { CurrentMouesPos.x - PrevMousePos.x, CurrentMouesPos.y - PrevMousePos.y, 0.f };
                    RootComponent->AddRotation({ DeltaMousePos.y / 20.f, DeltaMousePos.x / 20.f, 0.f });
                }
                else
                {
                    bControl = true;

                }

                PrevMousePos = CurrentMouesPos;
            }
            else
            {
                XMStoreFloat4x4(&RotMat, RootComponent->GetRotationMatrix());
                bControl = false;
            }
        }
    }

    if (auto DynamicMeshComp = Target->getComponent(ROOT_COMPONENT)->CastTo<DynamicMeshComponent>())
    {
        // 본 축 표시
        if (SelectedJointIndex != -1)
        {
            const Mat4 JointMat = DynamicMeshComp->GetJointMatrix(SelectedJointIndex);
            Vec3 Trans = {}, Rot = {}, Scale = {};
            DecomposeTransform(JointMat, Scale, Rot, Trans);
            getRenderer()->DrawCoordinate(W.get(), Trans, Rot);
        }

        if (auto Physics = DynamicMeshComp->GetDynamicMesh()->GetPhysics())
        {
            if (std::shared_ptr< MDynamicMeshPhysics> DynamicMeshPhysics = Physics->CastToShared<MDynamicMeshPhysics>())
            {
                for (FBodyCapsuleData& BodyCapsule : DynamicMeshPhysics->GetCapsules())
                {
                    const Mat4 JointMat = DynamicMeshComp->GetJointMatrix(BodyCapsule.AttachJointIndex);
                    Vec3 S, R, T;
                    DecomposeTransform(JointMat, S, R, T);

                    GetRenderer()->DrawCapsule(W.get(), BodyCapsule.GetRadius(), BodyCapsule.GetHalfHeight(), T, R);
                }
            }
        }
    }

    //Vec3 Forward = CameraComponent->GetForward();
    //Vec3 NewPos = {};
    //XMStoreFloat3(&NewPos, XMLoadFloat3(&Forward) * 2.f);
    //CameraComponent->setTranslation(NewPos);

    if (bOpen == false)
    {
        std::string t = Title;
        GetPostLoopDelegate().Add([t]() {
            GetEngine()->GetModule<MEditor>()->Editors.erase(t);
        });
        return;
    }
}

void MAssetEditor::RenderUI()
{
    MEditorBase::RenderUI();

    //if (ImGui::Begin("t1", &bOpen))
    //{
    //    const FTypeDesc* Current = AssetTypeDesc;
    //    while (Current)
    //    {
    //        DispatchType(Current, WorkingAsset.get());
    //        Current = Current->Parent;
    //    }

    //    // 애셋 타입에 따라 추가 처리
    //    if (std::shared_ptr<DynamicMesh> dynamicMesh = WorkingAsset->CastToShared<DynamicMesh>())
    //    {
    //        HandleDynamicMesh(dynamicMesh.get());
    //        HandleSkeleton(dynamicMesh->GetSkeleton().get(), nullptr);
    //    }

    //    else if (WorkingAsset->IsA<StaticMesh>())
    //    {
    //        if (ImGui::Button("Make ConvexHull Collision"))
    //        {
    //            GetPhysics()->SaveTest(std::static_pointer_cast<StaticMesh>(WorkingAsset));
    //        }

    //        if (ImGui::Button("Make MeshShape Collision"))
    //        {
    //            GetPhysics()->SaveTest(std::static_pointer_cast<StaticMesh>(WorkingAsset));
    //        }
    //    }

    //}
    //ImGui::End();
}

void MAssetEditor::HandleDynamicMesh(DynamicMesh* InDynamicMesh)
{
    if (ImGui::Button("Make Physics") && InDynamicMesh->GetPhysics() == nullptr)
    {
        std::wstring Path = InDynamicMesh->GetAssetPath();
        Path += TEXT("Asd");

        //OpenEditor(InDynamicMesh, MDynamicMeshPhysics::GetTypeDescStatic(), Path);
    }
}

void MAssetEditor::HandleSkeleton(MSkeleton* InSkeleton, std::function<void(uint32)> InContextMenu)
{
    // 본 표시
    std::function<void(int32)> DrawTree = [&](int32 InParentIndex) {
        const auto& ChildJoints = InSkeleton->GetChildJoints(InParentIndex);
        for (const auto& ChildJoint : ChildJoints)
        {
            int32 JointIndex = InSkeleton->GetJointIndex(ChildJoint.Name);
            if (InSkeleton->GetChildJoints(JointIndex).empty())
            {
                if (ImGui::TreeNodeEx(ChildJoint.Name.c_str(), ImGuiTreeNodeFlags_Leaf))
                {
                    if (ImGui::IsItemClicked())
                    {
                        SelectedJointIndex = JointIndex;
                    }

                    if (InContextMenu != nullptr)
                    {
                        InContextMenu(JointIndex);
                    }
                    ImGui::TreePop();
                }
            }
            else
            {
                if (ImGui::TreeNode(ChildJoint.Name.c_str()))
                {
                    if (ImGui::IsItemClicked())
                    {
                        SelectedJointIndex = JointIndex;
                    }

                    if (InContextMenu != nullptr)
                    {
                        InContextMenu(JointIndex);
                    }

                    DrawTree(JointIndex);
                    ImGui::TreePop();
                }
            }
        }
    };

    DrawTree(-1);
}
