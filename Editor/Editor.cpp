#include "Editor.h"
#include "MoonEngine.h"

#include "Window.h"

#include "Core/ResourceManager.h"
#include "Core/Physics/Physics.h"
#include "Core/ObjectPath.h"
#include "Core/Asset.h"

#include "World.h"
#include "Renderer.h"
#include "GraphicDevice.h"
#include "Texture.h"
#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "PointLightComponent.h"
#include "SphereComponent.h"
#include "StaticMeshComponent.h"
#include "Camera.h"
#include "Player.h"
#include "DirectInput.h"
#include "DynamicMeshComponent.h"
#include "Material.h"

#include "GameFramework/StaticMeshActor/StaticMeshActor.h"
#include "GameFramework/PointLightActor/PointLightActor.h"
#include "GameFramework/DirectionalLightActor/DirectionalLightActor.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"

// FBX
#include "FBXLoader.h"

#include "Source/Editor/EditorBase.h"

#define UseGround 1
#define UseDirectionalLight 1

using namespace DirectX;

const ImVec4 HighlightColor = { 1.f, 1.f, 0.f, 1.f };

MEditor::MEditor()
{
}

MEditor::~MEditor()
{
}

bool MEditor::Initialize()
{
    Super::Initialize();

    return true;
}

void MEditor::Update()
{
    std::shared_ptr<MWorld> World = GetWorld<MWorld>();

    if (auto CameraComponent = GetMainWorld()->getMainCamera()->getComponent(TEXT("RootComponent")))
    {
        float DeltaTime = World->getDeltaTime();
        Vec3 trans = CameraComponent->getTranslation();
        Vec3 look = CameraComponent->GetForward();
        Vec3 right = CameraComponent->getRight();
        float speed = CameraSpeedScale * DeltaTime;

        if (InputManager::keyPress(DIK_LSHIFT))
        {
            speed *= 5.f;
        }

        if (InputManager::keyPress(DIK_W))
        {
            trans.x += look.x * speed;
            trans.y += look.y * speed;
            trans.z += look.z * speed;
        }
        else if (InputManager::keyPress(DIK_S))
        {
            trans.x -= look.x * speed;
            trans.y -= look.y * speed;
            trans.z -= look.z * speed;
        }
        else if (InputManager::keyPress(DIK_D))
        {
            trans.x += right.x * speed;
            trans.y += right.y * speed;
            trans.z += right.z * speed;
        }
        else if (InputManager::keyPress(DIK_A))
        {
            trans.x -= right.x * speed;
            trans.y -= right.y * speed;
            trans.z -= right.z * speed;
        }

        if (GetMainWorld()->IsMouseInViewport())
        {
            CameraSpeedScale += static_cast<float>(InputManager::mouseMove(MOUSEAXIS::Z)) / 10.f;
        }
        CameraSpeedScale = CameraSpeedScale >= 1.f ? CameraSpeedScale : 1.f;

        CameraComponent->setTranslation(trans);

        if (InputManager::mousePress(MOUSEBUTTON::RB))
        {
            Vec3 CameraRot = CameraComponent->getRotation();
            Vec3 TargetRot = CameraRot;
            float mouseX = static_cast<float>(InputManager::mouseMove(MOUSEAXIS::X));
            float mouseY = static_cast<float>(InputManager::mouseMove(MOUSEAXIS::Y));

            TargetRot.x += mouseY * DeltaTime * 0.2f;
            TargetRot.y += mouseX * DeltaTime * 0.2f;

            float t = 0.5f;
            CurrentRot.x = ((1.f - t) * CurrentRot.x) + (t * TargetRot.x);
            CurrentRot.y = ((1.f - t) * CurrentRot.y) + (t * TargetRot.y);
            CurrentRot.z = ((1.f - t) * CurrentRot.z) + (t * TargetRot.z);

            CameraComponent->setRotation(CurrentRot);
        }
    }

    if (InputManager::keyDown(DIK_1))
    {
        GizmoMode = EGizmoMode::Trans;
    }
    if (InputManager::keyDown(DIK_2))
    {
        GizmoMode = EGizmoMode::Rot;
    }
    if (InputManager::keyDown(DIK_3))
    {
        GizmoMode = EGizmoMode::Scale;
    }

    if (InputManager::mouseDown(MOUSEBUTTON::LB) && IsPickable())
    {
        FHitData HitData = {};
        std::vector<FPrimitiveData> GizmoPrimitiveDatas;
        getRenderer()->GizmoMeshComp->GetPrimitiveData(GizmoPrimitiveDatas);

        if (World->Raycast(GizmoPrimitiveDatas, HitData))
        {
            if (bControlGizmo == false)
            {
                bSetGizmoOffset = true;
                bControlGizmo = true;
            }

            switch (HitData.PrimitiveIndex)
            {
            case 0:
                GizmoAxis = EAxies::Z;
                break;
            case 1:
                GizmoAxis = EAxies::Y;
                break;
            case 2:
                GizmoAxis = EAxies::X;
                break;
            }
        }
        else
        {
            bControlGizmo = false;
            if (World->Raycast(getRenderer()->GetRenderablePrimitiveData(), HitData))
            {
                std::shared_ptr<SceneComponent> Temp = HitData.HitComponent.lock()->CastTo<SceneComponent>();
                SetClickedComp(Temp);
            }
        }
    }

    if (ClickedComp.expired() == false)
    {
        getRenderer()->bGizmo = true;
        getRenderer()->GizmoPos = std::static_pointer_cast<MPrimitiveComponent>(ClickedComp.lock())->getWorldTranslation();
    }

    if (bControlGizmo && InputManager::mouseUp(MOUSEBUTTON::LB))
    {
        bControlGizmo = false;
    }

    if (auto GizmoTargetComp = std::static_pointer_cast<MPrimitiveComponent>(ClickedComp.lock()))
    {
        getRenderer()->GizmoPos = GizmoTargetComp->getWorldTranslation();

        if (bControlGizmo)
        {
            Vec3 Near = {};
            Vec3 Far = {};
            Vec2 Current = GetMainWindow()->GetMousePos();
            World->ScreenToWorld(Current, 0.f, Near);
            World->ScreenToWorld(Current, 1.f, Far);

            XMVECTOR Plane = XMVectorZero();
            Vec3 Pos = GizmoTargetComp->getWorldTranslation();
            switch (GizmoAxis)
            {
            case EAxies::X:
                Plane = XMPlaneFromPoints(XMLoadFloat3(&Pos), XMLoadFloat3(&Pos) + XMVectorSet(1.f, 0.f, 1.f, 0.f), XMLoadFloat3(&Pos) - XMLoadFloat3(&VEC3RIGHT));
                break;
            case EAxies::Z:
                Plane = XMPlaneFromPoints(XMLoadFloat3(&Pos), XMLoadFloat3(&Pos) + XMVectorSet(1.f, 0.f, 1.f, 0.f), XMLoadFloat3(&Pos) - XMLoadFloat3(&VEC3RIGHT));
                break;
            case EAxies::Y:
                Plane = XMPlaneFromPoints(XMLoadFloat3(&Pos), XMLoadFloat3(&Pos) + XMVectorSet(0.f, 1.f, -1.f, 0.f), XMLoadFloat3(&Pos) - XMLoadFloat3(&VEC3UP));
                break;
            }

            XMVECTOR HitPos = XMPlaneIntersectLine(Plane, XMLoadFloat3(&Near), XMLoadFloat3(&Far));
            if (bSetGizmoOffset)
            {
                XMStoreFloat3(&Prev, HitPos);
                bSetGizmoOffset = false;
            }

            switch (GizmoMode)
            {
                case EGizmoMode::Trans:
                {
                    Vec3 DeltaTrans = {};
                    XMStoreFloat3(&DeltaTrans, HitPos - XMLoadFloat3(&Prev));

                    switch (GizmoAxis)
                    {
                    case EAxies::X:
                        DeltaTrans.y = DeltaTrans.z = 0.f;
                        break;
                    case EAxies::Y:
                        DeltaTrans.x = DeltaTrans.z = 0.f;
                        break;
                    case EAxies::Z:
                        DeltaTrans.x = DeltaTrans.y = 0.f;
                        break;
                    }

                    GizmoTargetComp->AddTranslation(DeltaTrans);
                }
                break;
                case EGizmoMode::Rot:
                {
                    Vec3 DeltaRot = {};
                    XMStoreFloat3(&DeltaRot, HitPos - XMLoadFloat3(&Prev));

                    switch (GizmoAxis)
                    {
                    case EAxies::X:
                        DeltaRot.y = DeltaRot.z = 0.f;
                        break;
                    case EAxies::Y:
                        DeltaRot.x = DeltaRot.z = 0.f;
                        break;
                    case EAxies::Z:
                        DeltaRot.x = DeltaRot.y = 0.f;
                        break;
                    }
                    GizmoTargetComp->AddRotation(DeltaRot);
                }
                break;
                case EGizmoMode::Scale:
                {
                    Vec3 DeltaScale = {};
                    XMStoreFloat3(&DeltaScale, (HitPos - XMLoadFloat3(&Prev)) / 30.f);

                    switch (GizmoAxis)
                    {
                    case EAxies::X:
                        DeltaScale.y = DeltaScale.z = 0.f;
                        break;
                    case EAxies::Y:
                        DeltaScale.x = DeltaScale.z = 0.f;
                        break;
                    case EAxies::Z:
                        DeltaScale.x = DeltaScale.y = 0.f;
                        break;
                    }
                    GizmoTargetComp->AddScale(DeltaScale);
                }
            }

            XMStoreFloat3(&Prev, HitPos);
        }
    }

    if (InputManager::keyDown(DIK_ESCAPE))
    {
        ClickedComp.reset();
    }
}

void MEditor::Render()
{
    std::shared_ptr<MWorld> World = GetWorld<MWorld>();

    ImGui::Begin("World");

    auto SelectedComp = ClickedComp.expired() ? nullptr : ClickedComp.lock()->CastTo<StaticMeshComponent>();
    if (ImGui::CollapsingHeader("Test Functions"))
    {
        if (ImGui::CollapsingHeader("Actor") && SelectedComp)
        {
            auto IsNotEqual = [](float lhs, float rhs)->bool {
                return std::fabsf(lhs - rhs) > 0.00001;
            };

            //ImGui::SliderFloat("ForceY", &Force, 0.f, 10000.f);
            //if (ImGui::Button("AddForce"))
            //{
            //    SelectedComp->Temp(Force);
            //}

            if (ImGui::Button("ResetVelocity"))
            {
                SelectedComp->SetVelocity(0.f, 0.f, 0.f);
                SelectedComp->SetAngularVelocity(0.f, 0.f, 0.f);
            }

            if (ImGui::Button("ResetPos"))
            {
                SelectedComp->setTranslation(0.f, 5.f, 0.f);
            }
        }

        // FBX 로드 
        if (ImGui::CollapsingHeader("LoadFBX"))
        {
            static bool bMesh = false;
            static bool bMaterial = false;
            static bool bSkeleton = false;
            static bool bAnim = false;

            ImGui::Checkbox("Mesh", &bMesh);
            ImGui::SameLine(100.f);
            ImGui::Checkbox("Material", &bMaterial);
            ImGui::SameLine(200.f);
            ImGui::Checkbox("Skeleton", &bSkeleton);
            ImGui::SameLine(300.f);
            ImGui::Checkbox("Anim", &bAnim);
            ImGui::NewLine();

            if (ImGui::Button("Load"))
            {
                TCHAR FileName[256] = {};

                OPENFILENAMEW t = {};
                t.lStructSize = sizeof(t);
                t.hwndOwner = NULL;
                t.hInstance = NULL;
                t.lpstrFilter = TEXT("FBX 파일\0*.fbx");
                t.lpstrFile = FileName;
                t.nMaxFile = 256;
                t.lpstrInitialDir = TEXT(".");
                t.lpstrTitle = TEXT("Load FBX");

                if (GetOpenFileNameW(&t))
                {
                    MFBXLoader FBXLoader;
                    FBXLoader.SaveJsonAsset(FileName, bMesh, bMaterial, bSkeleton, bAnim);
                    wcout << FileName << endl;
                }
            }
        }

        if (ImGui::Button("Jolt Save"))
        {
            if (std::shared_ptr<MMeshComponent> MeshComp = ClickedComp.lock()->CastTo<MMeshComponent>())
            {
                if (MeshComp->GetMesh())
                {
                    GetPhysics()->SaveTest(MeshComp->GetMesh());
                }
            }
        }

        if (ImGui::Button("Jolt Load"))
        {
            if (std::shared_ptr<MMeshComponent> MeshComp = ClickedComp.lock()->CastTo<MMeshComponent>())
            {
                if (MeshComp->GetMesh())
                {
                    GetPhysics()->LoadTest();
                }
            }
        }
    }

    if (ImGui::CollapsingHeader("Level"))
    {
        ImGui::Indent(20);
        if (ImGui::Button("Save As"))
        {
            SaveAs(*GetMainWorld());
        }
        if (ImGui::Button("Load"))
        {
            GetLevelChangedDelegate().Broadcast();

            Open([&](const TCHAR* InFileName) {
                std::wstring FileName = InFileName;
                GetPostLoopDelegate().Add([FileName]() {
                    GetMainWorld()->GetActors().clear();
                    MJsonDeserializer Deserializer;
                    Deserializer.Deserialize(GetMainWorld(), FileName);
                    });
                });
        }
        ImGui::Indent(-20);
    }

    // 게임
    if (ImGui::CollapsingHeader("Game"))
    {
        ImGui::Indent(20);
        if (ImGui::Button("Play"))
        {
            World->PlayGame();
        }
        ImGui::Indent(-20);
    }

    // 기즈모 컨트롤
    for (int i = 0; i < (int)EGizmoMode::Count; ++i)
    {
        bool bHighlight = (int)GizmoMode == i;

        if (bHighlight)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, HighlightColor);
        }

        std::array<std::string, 3> Names = { "Translation", "Rotation", "Scale" };

        if (ImGui::Button(Names[i].c_str()))
        {
            GizmoMode = (EGizmoMode)i;
        }

        if (bHighlight)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
    }
    ImGui::NewLine();

    // 렌더러
    if (ImGui::CollapsingHeader("Render"))
    {
        ImGui::Text("Toatal primitive:%d", getRenderer()->TotalPrimitiveNum);
        ImGui::Text("show primitive:%d", getRenderer()->ShownPrimitiveNum);
        ImGui::Text("culled primitive:%d", getRenderer()->CulledPrimitiveNum);
        ImGui::Text("Frame: %d", GetMainWorld()->getFrame());

        const FTypeDesc* Current = getRenderer()->GetTypeDesc();
        while (Current)
        {
            DispatchStruct(Current, getRenderer().get());
            Current = Current->Parent;
        }
    }
    
    // 하이어라키
    if (ImGui::CollapsingHeader("Hierarchy"))
    {
        auto& Actors = World->GetActors();
        uint32 Num = GetSize(Actors);

        if (std::shared_ptr<Component> Comp = ClickedComp.lock())
        {
            // 하이라이트, Tick에서 계속 순회하는 것 보다는 변경 시 업데이트 해주는게 나을듯?
            for (auto& [Name, Actor] : Actors)
            {
                bool bHighlight = Comp->getOwningActor() == Actor;

                if (bHighlight)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                }

                if (ImGui::Selectable(Name.c_str()))
                {
                    SetClickedComp(Actor->getComponent(ROOT_COMPONENT));
                }

                if (bHighlight)
                {
                    ImGui::PopStyleColor(1);
                }
            }
        }


        // 액터 생성       
        auto& TypeDescs = GetTypeDescs();
        static const char* ActorClassName = nullptr;
        if (ImGui::BeginCombo("Actor Class", ActorClassName))
        {
            for (auto& [Name, TypeDesc] : TypeDescs)
            {
                if (TypeDesc->IsA<MActor>() == false)
                {
                    continue;
                }

                if (ImGui::Selectable(Name.c_str()))
                {
                    ActorClassName = Name.c_str();
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("Add Actor") && TypeDescs.find(ActorClassName) != TypeDescs.end())
        {
            std::shared_ptr<MActor> NewActor = CreateActor(GetMainWorld(), TypeDescs[ActorClassName]);
            ClickedComp = NewActor->getComponent(ROOT_COMPONENT);
        }
    }

    // 액터 편집 기능
    if (std::shared_ptr<Component> Comp = ClickedComp.lock())
    {
        if (std::shared_ptr<MActor> Actor = Comp->getOwningActor())
        {
            if (ImGui::CollapsingHeader("Actor Edit"))
            {
                for (auto& [Name, Comp] : Actor->GetComponents())
                {
                    char CName[128];
                    WStringToString(Name, CName, 128);
                    std::string ClassName = "(" + Comp->GetTypeDesc()->Name + ")";
                    strcat_s(CName, 128, ClassName.c_str());

                    bool bHighlight = ClickedComp.lock() == Comp;
                    if (bHighlight)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                    }

                    if (ImGui::Selectable(CName))
                    {
                        ClickedComp = Comp;
                    }

                    if (bHighlight)
                    {
                        ImGui::PopStyleColor(1);
                    }
                }

                // 컴포넌트 속성 편집 기능
                if (std::shared_ptr<MPrimitiveComponent> HitComponent = std::static_pointer_cast<MPrimitiveComponent>(ClickedComp.lock()))
                {
                    const FTypeDesc* Current = HitComponent->GetTypeDesc();
                    while (Current)
                    {
                        if (ImGui::CollapsingHeader(Current->Name.c_str()))
                        {
                            DispatchStruct(Current, HitComponent.get());
                        }

                        Current = Current->Parent;
                    }
                }
            }

            Actor->update(0.f);
        }

    }

    for (auto& [Title, TempEditor] : Editors)
    {
        TempEditor->Update();
    }

	ImGui::End();
}

bool MEditor::IsPickable() const
{
    return ImGui::GetIO().WantCaptureMouse == false && GetMainWorld()->IsMouseInViewport();
}

void MEditor::SetClickedComp(std::shared_ptr<SceneComponent>& InComp)
{
    const std::shared_ptr<SceneComponent> Old = ClickedComp.lock();
    if (Old != InComp)
    {
        if (Old)
        {
            OutLine(Old->getOwningActor(), false);
        }

        if (InComp)
        {
            OutLine(InComp->getOwningActor(), true);
        }

        ClickedComp = InComp;
    }
}

void MEditor::OnClickedCompChanged()
{
}

void MEditor::OutLine(std::shared_ptr<MActor>& InActor, bool bOutLine)
{
    if (InActor == nullptr)
    {
        return;
    }

    for (auto& [Name, Comp] : InActor->GetComponents())
    {
        if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = Comp->CastTo<MPrimitiveComponent>())
        {
            PrimitiveComp->SetStencil(bOutLine);
        }
    }
}

void DispatchContainer(const FTypeDesc* InElementTypeDesc, FVectorPropertyDesc* InContainerDesc, void* InObject)
{
    if (InObject == nullptr)
    {
        return;
    }

    ImGui::PushID(InContainerDesc->GetAsVoid(InObject));

    if (ImGui::CollapsingHeader(InContainerDesc->GetDisplayName().c_str()))
    {
        uint32 ElementNum = static_cast<uint32>(InContainerDesc->GetNum(InObject));

        // 추가 버튼
        if (ImGui::Button("Add"))
        {
            InContainerDesc->Resize(InObject, ElementNum + 1);
        }

        // 컨테이너 요소들 표시
        for (uint32 i = 0; i < ElementNum; ++i)
        {
            if (InContainerDesc->IsA<MAsset>())
            {
                auto Asset = *static_cast<std::shared_ptr<MAsset>*>(InContainerDesc->Get(InObject, i));
                std::string Path = Asset == nullptr ? "" : WStringToString(Asset->GetAssetPath());
                ImGui::Text(Path.c_str());

                const FTypeDesc* AssetTypeDesc = nullptr;
                if (Asset)
                {
                    AssetTypeDesc = Asset->GetTypeDesc();
                }
                else
                {
                    AssetTypeDesc = InContainerDesc->TypeDesc;
                }

                std::wstring Filter;
                if (AssetTypeDesc->IsA<MTexture>())
                {
                    Filter = TEXT("텍스쳐\0*.png\0");
                }
                else
                {
                    Filter = TEXT("애셋\0*.json\0");
                }

                ImGui::PushID(reinterpret_cast<int>(InObject) + i);
                ImGui::SameLine(300);
                if (ImGui::Button("Edit"))
                {
                    EditAsset(InObject, AssetTypeDesc, StringToWString(Path));
                }

                ImGui::SameLine(350);
                if (ImGui::Button("..."))
                {
                    TCHAR FileName[256] = {};

                    OPENFILENAMEW t = {};
                    t.lStructSize = sizeof(t);
                    t.hwndOwner = NULL;
                    t.hInstance = NULL;
                    t.lpstrFilter = Filter.c_str();
                    t.lpstrFile = FileName;
                    t.nMaxFile = 256;
                    t.lpstrInitialDir = TEXT(".");
                    t.lpstrTitle = TEXT("Load Asset");

                    if (GetOpenFileNameW(&t))
                    {
                        if (std::shared_ptr<MAsset> Asset = g_ResourceManager->Load(FileName, AssetTypeDesc))
                        {
                            void* AssetPtr = &Asset;
                            InContainerDesc->Set(InObject, i, AssetPtr);
                        }
                    }
                }

                ImGui::PopID();
                ImGui::NewLine();
            }
            else
            {
                void* ContainerElement = InContainerDesc->Get(InObject, i);
                std::string DisplayNameStr = std::to_string(i);
                const char* DisplayName = DisplayNameStr.c_str();

                if (InElementTypeDesc)
                {
                    DispatchStruct(InElementTypeDesc, ContainerElement);
                }
                else
                {
                    HandleProperty(InContainerDesc->Type, DisplayName, ContainerElement);
                }
            }
        }
    }

    ImGui::PopID();

}

void DispatchArray(const FTypeDesc* InElementTypeDesc, FPropertyDesc* InPropertyDesc, void* InObject)
{
    if (InObject == nullptr)
    {
        return;
    }

    if (ImGui::CollapsingHeader(InPropertyDesc->GetDisplayName().c_str()))
    {
        uint32 Num = static_cast<uint32>(InPropertyDesc->Num);

        // Array는 추가, 삭제할 수가 없는 고정된 사이즈임

        // Array 요소 표시
        for (uint32 i = 0; i < Num; ++i)
        {
            if (InPropertyDesc->IsA<MAsset>())
            {

            }
            else
            {
                void* ArrayElem = InPropertyDesc->GetAsVoid(InObject, i);
                std::string DisplayNameStr = InPropertyDesc->Name + std::to_string(i);
                const char* DisplayName = DisplayNameStr.c_str();

                if (InElementTypeDesc)
                {
                    DispatchStruct(InElementTypeDesc, ArrayElem);
                }
                else
                {
                    HandleProperty(InPropertyDesc->Type, DisplayName, ArrayElem);
                }
            }
        }
    }
}

void DispatchStruct(const FTypeDesc* InStructDesc, void* InObject)
{
    if (InObject == nullptr)
    {
        return;
    }

    for (FPropertyDesc* Prop : InStructDesc->Properties)
    {
        if (Prop->IsContainer())
        {
            DispatchContainer(Prop->TypeDesc, static_cast<FVectorPropertyDesc*>(Prop), InObject);
        }
        else if (Prop->IsArray()) // 배열
        {
            DispatchArray(Prop->TypeDesc, Prop, InObject);
        }
        else
        {
            if (Prop->TypeDesc == nullptr)
            {
                HandleProperty(Prop->Type, Prop->Name.c_str(), Prop->GetAsVoid(InObject));
            }
            else if (Prop->IsA<MAsset>())
            {
                ImGui::PushID(Prop);

                std::shared_ptr<MAsset> Asset = *static_cast<std::shared_ptr<MAsset>*>(Prop->GetAsVoid(InObject));

                ImGui::Text(Prop->GetDisplayName().c_str());

                ImGui::SameLine(100);
                std::string Path = Asset == nullptr ? "Empty" : WStringToString(Asset->GetAssetPath());
                ImGui::Text(Path.c_str());
                    
                ImGui::SameLine(300);
                if (ImGui::Button("Edit"))
                {
                    EditAsset(InObject, Prop->TypeDesc, StringToWString(Path));
                }

                ImGui::SameLine(350);

                if (ImGui::Button("..."))
                {
                    TCHAR FileName[256] = {};

                    OPENFILENAMEW t = {};
                    t.lStructSize = sizeof(t);
                    t.hwndOwner = NULL;
                    t.hInstance = NULL;
                    t.lpstrFilter = TEXT("json 파일\0*.json");
                    t.lpstrFile = FileName;
                    t.nMaxFile = 256;
                    t.lpstrInitialDir = TEXT(".");
                    t.lpstrTitle = TEXT("Load FBX");

                    if (GetOpenFileNameW(&t))
                    {
                        wcout << FileName << endl;

                        // Asset 부분만 불러와 Path를 세팅하도록
                        std::shared_ptr<MAsset> NewAsset = g_ResourceManager->Load(FileName, Prop->TypeDesc);
                        static_cast<FFundamentalPropertyDesc<MAsset>*>(Prop)->Set(InObject, NewAsset);
                    }
                }
                ImGui::PopID();

                ImGui::NewLine();
            }
            else
            {
                DispatchStruct(Prop->TypeDesc, Prop->GetAsVoid(InObject));
            }
        }
    }
}

void HandleProperty(EType InType, const char* DisplayName, void* InData)
{
    switch (InType)
    {
    case EType::Int:
    case EType::Enum:
    {
        ImGui::InputInt(DisplayName, static_cast<int*>(InData));
    }
    break;
    case EType::Float:
    {
        ImGui::InputFloat(DisplayName, static_cast<float*>(InData));
    }
    break;
    case EType::Bool:
    {
        ImGui::Checkbox(DisplayName, static_cast<bool*>(InData));
    }
    break;
    case EType::Vec2:
    case EType::Vec4:
    {

    }
    break;
    case EType::Vec3:
    {
        auto Temp = static_cast<Vec3*>(InData);
        float TempArr[3] = { Temp->x, Temp->y, Temp->z };
        if (ImGui::InputFloat3(DisplayName, TempArr))
        {
            *Temp = { TempArr[0], TempArr[1], TempArr[2] };
        }
    }
    break;
    case EType::WString:
    {
        auto Temp = static_cast<std::wstring*>(InData);
        char Buff[256] = {};
        WStringToString(*Temp, Buff, 256);

        if (ImGui::InputText(DisplayName, Buff, 256))
        {

        }
    }
    break;
    }
}

void EditAsset(void* InObject, const FTypeDesc* InAssetTypeDesc, const std::wstring& InPath)
{
    if (InPath.empty())
    {
        return;
    }

    std::shared_ptr<MAsset> AssetCopy = std::shared_ptr<MAsset>(static_cast<MAsset*>(Create(InAssetTypeDesc)));
    MJsonDeserializer Deserializer;
    Deserializer.Deserialize(AssetCopy, InPath);

    std::shared_ptr<MAssetEditor> a = std::make_shared<MAssetEditor>(InObject, InAssetTypeDesc, AssetCopy);
    std::string Title = a->GetTitle();
    //a->GetClosedDelegate().Add([Title]() {
    //    GetEngine()->GetModule<MEditor>()->Editors.erase(Title);
    //});

    GetEngine()->GetModule<MEditor>()->Editors.emplace(Title, a);
}
