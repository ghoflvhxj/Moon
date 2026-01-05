#include "Editor.h"
#include "MoonEngine.h"

#include "Window.h"

#include "Core/ResourceManager.h"
#include "Core/ObjectPath.h"
#include "Core/Asset.h"

#include "Module/Physics/Physics.h"
#include "Module/Physics/CharacterPhysics.h"

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

#include "Editor/AssetEditor.h"
#include "Editor/ActorEditor.h"
#include "Editor/DynamicMeshPhysicsEditor.h"
#include "Editor/MainWindow.h"
#include "Renderer/EditorPass.h"

using namespace DirectX;

//const ImVec4 HighlightColor = { 1.f, 1.f, 0.f, 1.f };

MEditor::MEditor()
{
}

MEditor::~MEditor()
{
}

bool MEditor::Initialize()
{
    Super::Initialize();

    GizmoMesh = g_ResourceManager->Load(TEXT("Base/Gizmo.json"), StaticMesh::GetTypeDescStatic())->CastToShared<StaticMesh>();

    if (getRenderer())
    {
        getRenderer()->AddRenderPass(ERenderPass::CustomPass0, MRenderer::CreateRenderPass<MEditorPass>());
    }
    GetMainWindow()->GetOnViewportSizeChangedDelegate().Add(this, [&](uint32 a, uint32 b, uint32 c, uint32 d, uint32 e, bool){
        for (auto& [Name, Actor] : GetMainWorld()->GetActors())
        {
            Actor->update(0.f);
        }
    });
    return true;
}

void MEditor::Release()
{
    Super::Release();

    GizmoMesh.reset();
    Editors.clear();
}

void MEditor::Update()
{
    std::shared_ptr<MWorld> World = GetMainWorld();

    if (World->IsMouseInViewport() && World->IsForegorund())
    {
        if (auto CameraComponent = World->getMainCamera()->getComponent(TEXT("RootComponent")))
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

            CameraSpeedScale += static_cast<float>(InputManager::mouseMove(EAxis::Z)) / 10.f;
            CameraSpeedScale = std::max(CameraSpeedScale, 1.f);

            CameraComponent->setTranslation(trans);

            Vec3 CameraRot = CameraComponent->GetWorldRotation();

            //if (InputManager::mousePress(MOUSEBUTTON::RB))
            //{
            //    float mouseX = static_cast<float>(InputManager::mouseMove(EAxis::X)) * 0.3f;
            //    float mouseY = static_cast<float>(InputManager::mouseMove(EAxis::Y)) * 0.3f;

            //    TargetRot.x += mouseY;
            //    TargetRot.y += mouseX;
            //    TargetRot = Wind(TargetRot);
            //}

            //cout << CameraRot << endl;

            //Vec3 DeltaRot = {};
            //XMStoreFloat3(&DeltaRot, (XMLoadFloat3(&TargetRot) - XMLoadFloat3(&CameraRot)) * DeltaTime);

            //CameraComponent->AddRotation(DeltaRot);

            if (InputManager::mousePress(MOUSEBUTTON::RB))
            {
                float mouseX = static_cast<float>(InputManager::mouseMove(EAxis::X));
                float mouseY = static_cast<float>(InputManager::mouseMove(EAxis::Y));

                AddRot.x += ToRadian(mouseY);
                AddRot.y += ToRadian(mouseX);
            }

            // AddRot이 0이여도 max로인해 PI가 적용된다
            //Vec3 Add = { AddRot.x * DeltaTime * 10.f, AddRot.y * DeltaTime * 10.f, 0.f };
            Vec3 Add = { std::clamp(AddRot.x, -PI, PI) * DeltaTime, std::clamp(AddRot.y, -PI, PI) * DeltaTime, 0.f };

            CameraComponent->AddRotation(Add);

            // 누적 회전에서 뺌
            AddRot.x -= AddRot.x * DeltaTime * 10.f;
            AddRot.y -= AddRot.y * DeltaTime * 10.f;

            //auto T = [](float& Value, float AddValue) {
            //    bool OldSign = GetSign(Value);
            //    Value -= AddValue;

            //    if (OldSign != GetSign(Value))
            //    {
            //        Value = 0.f;
            //    }
            //};

            //T(AddRot.x, Add.x);
            //T(AddRot.y, Add.y);

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

        if (InputManager::keyDown(DIK_ESCAPE))
        {
            ClickedComp.reset();
        }

        if (InputManager::mouseDown(MOUSEBUTTON::LB) && IsPickable())
        {
            auto& TemporalPrimitives = getRenderer()->GetScene(GetMainWorld()->GetID())->GetTemporalPrimitiveDatas();

            FHitData HitData = {};
            if (World->Raycast(TemporalPrimitives, HitData, (uint8)EPrimitiveType::CustomPrimitiveType0))
            {
                if (bControlGizmo == false)
                {
                    bSetGizmoOffset = true;
                    bControlGizmo = true;
                }

                switch (HitData.PrimitiveIndex)
                {
                case 0:
                    GizmoAxis = EAxis::Z;
                    break;
                case 1:
                    GizmoAxis = EAxis::Y;
                    break;
                case 2:
                    GizmoAxis = EAxis::X;
                    break;
                }
            }
            else
            {
                auto& Primitives = getRenderer()->GetScene(GetMainWorld()->GetID())->GetRenderablePrimitiveData();
                bControlGizmo = false;
                if (World->Raycast(Primitives, HitData, (uint8)EPrimitiveType::Mesh))
                {
                    std::shared_ptr<MSceneComponent> Temp = HitData.HitComponent.lock()->CastToShared<MSceneComponent>();
                    SetClickedComp(Temp);
                }
            }
        }

        if (bControlGizmo && InputManager::mouseUp(MOUSEBUTTON::LB))
        {
            bControlGizmo = false;
        }
    }

    if (auto GizmoTargetComp = std::static_pointer_cast<MPrimitiveComponent>(ClickedComp.lock()))
    {
        const Vec3& CameraPos = GetMainWorld()->getMainCamera()->GetWorldTranslation();
        const Vec3& GizmoPos = GizmoTargetComp->getTranslation();
        Vec3 Scale = { 0.001f, 0.001f, 0.001f };
        float DistToScale = XMVectorGetX(XMVector3Length(XMLoadFloat3(&CameraPos) - XMLoadFloat3(&GizmoPos))) / 10.f;

        Scale.x *= DistToScale;
        Scale.y *= DistToScale;
        Scale.z *= DistToScale;

        if (GizmoMesh.expired() == false)
        {
            getRenderer()->DrawPrimitive(GetMainWorld().get(), GizmoMesh.lock(), GizmoTargetComp->getTranslation(), VEC3ZERO, Scale, EPrimitiveType::CustomPrimitiveType0);
        }

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
            case EAxis::X:
                Plane = XMPlaneFromPoints(XMLoadFloat3(&Pos), XMLoadFloat3(&Pos) + XMVectorSet(1.f, 0.f, 1.f, 0.f), XMLoadFloat3(&Pos) - XMLoadFloat3(&VEC3RIGHT));
                break;
            case EAxis::Z:
                Plane = XMPlaneFromPoints(XMLoadFloat3(&Pos), XMLoadFloat3(&Pos) + XMVectorSet(1.f, 0.f, 1.f, 0.f), XMLoadFloat3(&Pos) - XMLoadFloat3(&VEC3RIGHT));
                break;
            case EAxis::Y:
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
                    case EAxis::X:
                        DeltaTrans.y = DeltaTrans.z = 0.f;
                        break;
                    case EAxis::Y:
                        DeltaTrans.x = DeltaTrans.z = 0.f;
                        break;
                    case EAxis::Z:
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
                    case EAxis::X:
                        DeltaRot.y = DeltaRot.z = 0.f;
                        break;
                    case EAxis::Y:
                        DeltaRot.x = DeltaRot.z = 0.f;
                        break;
                    case EAxis::Z:
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
                    case EAxis::X:
                        DeltaScale.y = DeltaScale.z = 0.f;
                        break;
                    case EAxis::Y:
                        DeltaScale.x = DeltaScale.z = 0.f;
                        break;
                    case EAxis::Z:
                        DeltaScale.x = DeltaScale.y = 0.f;
                        break;
                    }
                    GizmoTargetComp->AddScale(DeltaScale);
                }
            }

            GizmoTargetComp->getOwningActor()->update(0.f);

            XMStoreFloat3(&Prev, HitPos);
        }
    }

    for (auto& [Title, TempEditor] : Editors)
    {
        TempEditor->Update();
    }
}

void MEditor::Render()
{
}

void MEditor::Open(std::function<void(const TCHAR* InFileName)> InFunction, std::shared_ptr<MWindow> InOwner)
{
    TCHAR FileName[256] = {};
    OPENFILENAMEW OpenFileDesc = {};
    OpenFileDesc.hwndOwner = InOwner == nullptr ? GetMainWindow()->getHandle() : InOwner->getHandle();
    OpenFileDesc.lStructSize = sizeof(OpenFileDesc);
    OpenFileDesc.lpstrFilter = TEXT("json 파일\0*.json");
    OpenFileDesc.lpstrFile = FileName;
    OpenFileDesc.nMaxFile = MAX_PATH;
    OpenFileDesc.lpstrInitialDir = TEXT(".");
    OpenFileDesc.lpstrTitle = TEXT("파일 열기");
    OpenFileDesc.Flags = OFN_EXPLORER;

    if (GetOpenFileNameW(&OpenFileDesc))
    {
        InFunction(FileName);
    }
}

bool MEditor::IsPickable() const
{
    return ImGui::GetIO().WantCaptureMouse == false && GetMainWorld()->IsMouseInViewport();
}

void MEditor::SetClickedComp(std::shared_ptr<MSceneComponent>& InComp)
{
    std::shared_ptr<MSceneComponent> Old = ClickedComp.lock();
    if (Old != InComp)
    {
        if (Old)
        {
            OutLine(Old->getOwningActor().get(), false);
        }

        if (InComp)
        {
            OutLine(InComp->getOwningActor().get(), true);
        }

        ClickedComp = InComp;
    }
}

void MEditor::OnClickedCompChanged()
{
}

void MEditor::OutLine(MActor* InActor, bool bOutLine)
{
    if (InActor == nullptr)
    {
        return;
    }

    for (auto& [Name, Comp] : InActor->GetComponents())
    {
        if (std::shared_ptr<MPrimitiveComponent> PrimitiveComp = Comp->CastToShared<MPrimitiveComponent>())
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
            ImGui::PushID(i);

            if (InContainerDesc->IsA<MAsset>())
            {
                std::shared_ptr<MAsset>& Asset = *static_cast<std::shared_ptr<MAsset>*>(InContainerDesc->Get(InObject, i));
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

                ImGui::PushID(InObject);
                ImGui::SameLine(300);
                if (ImGui::Button("Edit"))
                {
                    OpenEditor(static_cast<MObject*>(InObject), Asset, StringToWString(Path));
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
                    DispatchType(InElementTypeDesc, ContainerElement);
                }
                else
                {
                    PropertyUI(InContainerDesc->Type, DisplayName, ContainerElement);
                }
            }

            ImGui::PopID();
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
                    DispatchType(InElementTypeDesc, ArrayElem);
                }
                else
                {
                    PropertyUI(InPropertyDesc->Type, DisplayName, ArrayElem);
                }
            }
        }
    }
}

void DispatchType(const FTypeDesc* InTypeDesc, void* InObject)
{
    if (InObject == nullptr)
    {
        return;
    }

    for (FPropertyDesc* Prop : InTypeDesc->Properties)
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
                PropertyUI(Prop->Type, Prop->Name.c_str(), Prop->GetAsVoid(InObject));
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
                    OpenEditor(static_cast<MObject*>(InObject), Asset, StringToWString(Path));
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
                        void* Temp = &NewAsset;
                        Prop->SetAsVoid(InObject, Temp);
                    }
                }
                ImGui::PopID();

                ImGui::NewLine();
            }
            else
            {
                if (Prop->bSharedPtr)
                {
                    std::shared_ptr<MObject> TempObject = *static_cast<std::shared_ptr<MObject>*>(Prop->GetAsVoid(InObject));
                    DispatchType(Prop->TypeDesc, TempObject.get());
                }
                else
                {
                    DispatchType(Prop->TypeDesc, Prop->GetAsVoid(InObject));
                }
            }
        }
    }
}

void DispatchType2(const FTypeDesc* InTypeDesc, void* InData)
{
    if (InData == nullptr || InTypeDesc == nullptr)
    {
        return;
    }

    while (InTypeDesc != nullptr)
    {
        if (ImGui::CollapsingHeader(InTypeDesc->Name.c_str()))
        {
            for (FPropertyDesc* Prop : InTypeDesc->Properties)
            {
                if (Prop->IsContainer())
                {
                    DispatchContainer(Prop->TypeDesc, static_cast<FVectorPropertyDesc*>(Prop), InData);
                }
                else if (Prop->IsArray()) // 배열
                {
                    DispatchArray(Prop->TypeDesc, Prop, InData);
                }
                else
                {
                    if (Prop->TypeDesc == nullptr)
                    {
                        PropertyUI(Prop->Type, Prop->Name.c_str(), Prop->GetAsVoid(InData));
                    }
                    else if (Prop->IsA<MAsset>())
                    {
                        ImGui::PushID(Prop);

                        std::shared_ptr<MAsset> Asset = *static_cast<std::shared_ptr<MAsset>*>(Prop->GetAsVoid(InData));
                        MObject* Object = Asset.get();

                        ImGui::Text(Prop->GetDisplayName().c_str());

                        ImGui::SameLine(100);
                        std::string Path = Asset == nullptr ? "Empty" : WStringToString(Asset->GetAssetPath());
                        ImGui::Text(Path.c_str());

                        ImGui::SameLine(300);
                        if (ImGui::Button("Edit"))
                        {
                            OpenEditor(Object, Asset, StringToWString(Path));
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
                                void* Temp = &NewAsset;
                                Prop->SetAsVoid(InData, Temp);
                            }
                        }
                        ImGui::PopID();

                        ImGui::NewLine();
                    }
                    else
                    {
                        if (Prop->bSharedPtr)
                        {
                            ImGui::Indent(20.f);
                            if (ImGui::CollapsingHeader(Prop->Name.c_str()))
                            {
                                ImGui::Indent(20.f);
                                std::shared_ptr<MObject> TempObject = *static_cast<std::shared_ptr<MObject>*>(Prop->GetAsVoid(InData));
                                DispatchType2(Prop->TypeDesc, TempObject.get());
                                ImGui::Indent(-20.f);
                            }
                            ImGui::Indent(-20.f);
                        }
                        else
                        {
                            DispatchType2(Prop->TypeDesc, Prop->GetAsVoid(InData));
                        }
                    }
                }
            }
        }

        InTypeDesc = InTypeDesc->Parent;
    }
}

void PropertyUI(EType InType, const char* DisplayName, void* InData)
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
        auto Temp = static_cast<float*>(InData);
        float Temp2 = *Temp;
        if (ImGui::InputFloat(DisplayName, &Temp2))
        {
            *Temp = Temp2;
        }
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

void OpenEditor(MObject* InOwner, std::shared_ptr<MObject> InObject, const std::wstring& InPath)
{
    if (InPath.empty())
    {
        return;
    }

    // 애셋의 사본을 만들어서 열기
    GetPostLoopDelegate().Add([InOwner, InObject, InPath]() {
        std::shared_ptr<MEditorBase> NewEditor = nullptr;
        if (InObject->IsA<MAsset>())
        {
            if (InObject->GetTypeDescStatic() == MDynamicMeshPhysics::GetTypeDescStatic())
            {
                NewEditor = std::make_shared<MDynamicMeshPhysicsEditor>(InOwner);
            }
            else
            {
                NewEditor = std::make_shared<MAssetEditor>(InOwner);
            }
        }

        NewEditor->SetObject(InObject);

        GetEngine()->GetModule<MEditor>()->Editors[NewEditor->GetTitle()] = NewEditor;
    });

    //std::shared_ptr<MAsset> AssetCopy = std::shared_ptr<MAsset>(static_cast<MAsset*>(Create(InAssetTypeDesc)));
    //AssetCopy->Load(InPath);

    //std::shared_ptr<MAssetEditor> NewAssetEditor = std::make_shared<MAssetEditor>(InAssetOwner);
    //// TODO. 하드 코딩을 제거하고 애셋에 맞는 에디터 인스턴스를 생성하도록
    //if (InAssetTypeDesc == MDynamicMeshPhysics::GetTypeDescStatic())
    //{
    //    NewAssetEditor = std::make_shared<MDynamicMeshPhysicsEditor>(InAssetOwner);
    //}

    //NewAssetEditor->SetAsset(AssetCopy);

    //GetEngine()->GetModule<MEditor>()->Editors.emplace(NewAssetEditor->GetTitle(), NewAssetEditor);
}

void OpenEditor(std::shared_ptr<MObject> InObject)
{
    if (InObject == nullptr)
    {
        return;
    }

    if (InObject->IsA<MActor>())
    {
        std::shared_ptr<MEditorBase> NewEditor = std::make_shared<MActorEditor>();
        NewEditor->SetObject(InObject);

        GetEngine()->GetModule<MEditor>()->Editors[NewEditor->GetTitle()] = NewEditor;
    }
}

MEditorBase::MEditorBase()
{
    WeakRenderer = GetEngine()->GetModule<MRenderer>();

    T = GetWindowManager()->AddWindow<MEditorBaseWindow>(StringToWString(Title), 300, 300, g_hWnd, TEXT("ShootingGame"));
    T->Initialize();
    T->GetOnImGuiRenderedDelegate().Add(this, &MEditorBase::RenderUI);

    W = std::make_shared<MWorld>();
    W->SetWorldType(EWorldType::WorkInEditor);
    W->Initialize();

    GetEngine()->AddWorld(W, T);

    //W->PlayGame();
    GetRenderer()->GetScene(W->GetID())->SetDrawCollision(true);
    Light = CreateActor<MDirectionalLightActor>(W);
}

bool MEditorBase::SetObject(std::shared_ptr<MObject> InObject)
{
    if (InObject == nullptr)
    {
        return false;
    }

    SourceObject = InObject;
    WorkingObject = DuplicateObject(InObject);

    HandleObject();

    return true;
}

void MEditorBase::RenderUI()
{
    if (ImGui::Begin(Title.c_str(), &bOpen))
    {
        if (ImGui::BeginMenu("File"))
        {
            bool bResult = false;
            if (ImGui::MenuItem("Save"))
            {
                MJsonSerializer Serializer;
                Serializer.Serialize(WorkingObject, GetPath(), true);
                bResult = true;
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
                    Serializer.Serialize(WorkingObject, FileName, true);
                    bResult = true;
                }
            }

            if (bResult)
            {
                WorkingObject->Copy(SourceObject.get());
                OnSaved();
            }

            ImGui::EndMenu();
        }

        DispatchType2(WorkingObject->GetTypeDesc(), WorkingObject.get());

        ImGui::End();
    }
}
