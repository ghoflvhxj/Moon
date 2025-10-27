#include "AssetEditor.h"
#include "MoonEngine.h"

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
#include "DynamicMeshComponent.h"
#include "GameFramework/DynamicMeshActor/DynamicMeshActor.h"
#include "GameFramework/DirectionalLightActor/DirectionalLightActor.h"

#include "WIndow.h"
#include "WindowManager.h"

#include <commdlg.h>

using namespace DirectX;

MAssetEditor::MAssetEditor(MObject* InObject)
    : AssetOwningObject(InObject)
{
    WeakJolt = GetEngine()->GetModule<MJoltPhysics>();
    WeakRenderer = GetEngine()->GetModule<MRenderer>();

    //Context = ImGui::CreateContext();
}

void MAssetEditor::SetAsset(std::shared_ptr<MAsset>& InAsset)
{
    if (InAsset == nullptr)
    {
        return;
    }

    Asset = InAsset;
    AssetTypeDesc = InAsset->GetTypeDesc();

    Title = AssetTypeDesc->Name + " Edit";

    T = GetWindowManager()->CreateWindow<MWindow>(TEXT("A"), 300, 300, g_hWnd, TEXT("ShootingGame"));
    W = std::make_shared<MWorld>();
    W->Initialize();
    GetEngine()->AddWorld(W, T);

    if (auto DA = CreateActor<MDynamicMeshActor>(W))
    {
        DA->SetDynamicMesh(InAsset->GetAssetPath());
        DA->SetWorldTranslation({ 0.f, 0.f, 3.f });
        DA->update(0.f);
    }

    Light = CreateActor<MDirectionalLightActor>(W);
}

void MAssetEditor::Update()
{
    if (Light)
    {
        //Light->getComponent(ROOT_COMPONENT)->AddRotation({ 0.1f, 0.f, 0.f });
        Light->update(0.f);
    }
    /*
    if (bOpen == false)
    {
        std::string t = Title;
        GetPostLoopDelegate().Add([t]() {
            GetEngine()->GetModule<MEditor>()->Editors.erase(t);
        });
        return;
    }

    if (ImGui::Begin(Title.c_str(), &bOpen))
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                MJsonSerializer Serializer;
                Serializer.Serialize(Asset, Asset->GetAssetPath(), true);
            }
            if (ImGui::MenuItem("Save As"))
            {
                wchar_t FileName[256] = {};
                OPENFILENAMEW OpenFile = {};
                OpenFile.lStructSize = sizeof(OPENFILENAMEW);
                OpenFile.lpstrFilter = TEXT("json파일\0*.json\0");
                OpenFile.lpstrFile = FileName;
                OpenFile.nMaxFile = MAX_PATH;
                OpenFile.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                OpenFile.lpstrDefExt = TEXT("json");
                if (GetSaveFileNameW(&OpenFile))
                {
                    MJsonSerializer Serializer;
                    Serializer.Serialize(Asset, FileName, true);
                }
            }

            ImGui::EndMenu();
        }

        const FTypeDesc* Current = AssetTypeDesc;
        while (Current)
        {
            DispatchStruct(Current, Asset.get());
            Current = Current->Parent;
        }

        // 애셋 타입에 따라 추가 처리
        if (std::shared_ptr<DynamicMesh> dynamicMesh = Asset->CastTo<DynamicMesh>())
        {
            HandleDynamicMesh(dynamicMesh.get());
            HandleSkeleton(dynamicMesh->GetSkeleton().get(), nullptr);
        }

        else if (Asset->IsA<StaticMesh>())
        {
            if (ImGui::Button("Make ConvexHull Collision"))
            {
                GetPhysics()->SaveTest(std::static_pointer_cast<StaticMesh>(Asset));
            }

            if (ImGui::Button("Make MeshShape Collision"))
            {
                GetPhysics()->SaveTest(std::static_pointer_cast<StaticMesh>(Asset));
            }
        }

        Test();

        ImGui::End();
    }
    */
}

void MAssetEditor::Render()
{
    // 여기서 하는 것 보다는 월드마다 컨텍스트가 존재하도록 하는 게?
    ImGuiContext* PrevContext = ImGui::GetCurrentContext();

    //ImGui::SetCurrentContext(Context);

    //ImGui::SetCurrentContext(PrevContext);
}

void MAssetEditor::HandleDynamicMesh(DynamicMesh* InDynamicMesh)
{
    if (ImGui::Button("Make Physics") && InDynamicMesh->GetPhysics() == nullptr)
    {
        std::wstring Path = InDynamicMesh->GetAssetPath();
        Path += TEXT("Asd");

        OpenAssetEditor(InDynamicMesh, MDynamicMeshPhysics::GetTypeDescStatic(), Path);
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

    /*
    // 본 축 표시
    if (auto DynamicMeshComp = AssetOwningObject->CastTo<DynamicMeshComponent>())
    {
        if (SelectedJointIndex != -1)
        {
            const Mat4 Matrix = DynamicMeshComp->GetJointMatrix(SelectedJointIndex);
            Vec3 Pos = {}, Rot = {}, Scale = {};
            DecomposeTransform(Matrix, Scale, Rot, Pos);
            getRenderer()->DrawCoordinate(W.get(), Pos, Rot, {0.3f, 0.3f, 0.3f});
        }
    }
    else if (auto _DynamicMesh = AssetOwningObject->CastTo<DynamicMesh>())
    {
        const FJoint& Joint = _DynamicMesh->GetJoint(SelectedJointIndex);
        //Vec3 Trans = { -Joint.Position.x / Joint.Scale.x, -Joint.Position.y / Joint.Scale.y, -Joint.Position.z / Joint.Scale.z };
        Vec3 Trans = { -Joint.Position.x / 3.54f, -Joint.Position.y / 3.54f, -Joint.Position.z / 3.54f };
        Vec3 Rot = ToRadian(Joint.Rotation);
        getRenderer()->DrawCoordinate(W.get(), Trans, Rot, { 0.3f, 0.3f, 0.3f });
    }
    */
}
