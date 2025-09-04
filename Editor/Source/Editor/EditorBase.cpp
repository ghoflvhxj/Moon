#include "EditorBase.h"
#include "MoonEngine.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

#include "Core/Asset.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"

// 모듈로 옮기기
#include "Core/Physics/Jolt/Jolt.h"
#include "Renderer.h"

#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "DynamicMeshComponent.h"

#include <commdlg.h>

MAssetEditor::MAssetEditor(void* InObject, const FTypeDesc* InTypeDesc, std::shared_ptr<MAsset>& InAsset)
    : Object(InObject)
    , AssetTypeDesc(InTypeDesc)
    , Asset(InAsset)
{
    Title = AssetTypeDesc->Name + " Edit";

    WeakJolt = GetEngine()->GetModule<MJoltPhysics>();
    WeakRenderer = GetEngine()->GetModule<MRenderer>();
}

void MAssetEditor::Update()
{
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
        if (Asset->IsA<DynamicMesh>())
        {
            // 조인트의 위치에 캡슐을 그리고 싶은데, 위치를 얻으려면 컴포넌트가 필요함...
            if (DynamicMeshComponent* DynamicMeshComp = static_cast<DynamicMeshComponent*>(Object))
            {
                std::shared_ptr<DynamicMesh> Dm = Asset->CastTo<DynamicMesh>();
                for (auto& BodyCapsuleData : Dm->BodyCapsuleDatas)
                {
                    const Vec3& JointPos = DynamicMeshComp->GetJointPosition(BodyCapsuleData.AttachJointIndex);
                    const Vec4& JointRot = DynamicMeshComp->GetJointRotation(BodyCapsuleData.AttachJointIndex);
                    if (BodyCapsuleData.PrimitiveID == -1)
                    {
                        BodyCapsuleData.PrimitiveID = GetRenderer()->MakeCapsule(1.f, 1.f);
                    }
                    GetRenderer()->UpdatePrimitive(BodyCapsuleData.PrimitiveID, JointPos, JointRot);
                }
            }

            if (ImGui::CollapsingHeader("Physics Body"))
            {
                //if (ImGui::Button("Add Capsule"))
                //{
                //    if (auto JoltPhysics = GetJolt())
                //    {
                //        //JoltPhysics->MakeCapsule();
                //    }
                //    if (auto Renderer = GetRenderer())
                //    {
                //        Renderer->MakeCapsule();
                //    }
                //}
            }
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

        ImGui::End();
    }
}
