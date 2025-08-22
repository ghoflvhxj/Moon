#include "MyGame.h"
#include "MoonEngine.h"

#include "Core/ResourceManager.h"
#include "Core/Physics/Physics.h"
#include "Core/ObjectPath.h"
#include "Core/Asset.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"

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
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Material.h"

#include "GameFramework/StaticMeshActor/StaticMeshActor.h"
#include "GameFramework/PointLightActor/PointLightActor.h"
#include "GameFramework/DirectionalLightActor/DirectionalLightActor.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

// 파일 다이얼로그
#include <commdlg.h>

// FBX
#include "FBXLoader.h"

#define UseGround 1
#define UseDirectionalLight 1

using namespace DirectX;

const ImVec4 HighlightColor = { 1.f, 1.f, 0.f, 1.f };

MyGame::MyGame()
	: MainGame()
	, _pPlayer{ nullptr }
{
}

MyGame::~MyGame()
{
}

const bool MyGame::initialize()
{
    MainGame::initialize();

	_pPlayer = CreateActor<Player>(GetShared());

    LanternActor = CreateActor<MStaticMeshActor>(GetShared());
    LanternActor->GetStaticMeshCompoent()->SetPhysicsType(EPhysicsType::Dynamic);
    LanternActor->SetStaticMesh(TEXT("Lantern/Lantern.json"));
    LanternActor->GetStaticMeshCompoent()->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
    LanternActor->GetStaticMeshCompoent()->SetDrawCollision(true);
    LanternActor->GetStaticMeshCompoent()->setDrawingBoundingBox(true);
    LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(false);
    LanternActor->GetStaticMeshCompoent()->RemovePhysics();

#if UseGround == 1
    auto Ground = CreateActor<MStaticMeshActor>(GetShared());
    Ground->GetStaticMeshCompoent()->SetMesh(TEXT("Base/Box.json"));
    
    std::shared_ptr<MTexture> Diffuse = nullptr;
    if (g_ResourceManager->Load(TEXT("./Resources/Texture/stone_01_albedo.jpg"), Diffuse))
    {
        //Ground->GetStaticMeshCompoent()->GetMesh()->getMaterial(0);
        Ground->GetStaticMeshCompoent()->GetMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, Diffuse);
    }
    Ground->GetStaticMeshCompoent()->GetMesh()->getMaterial(0)->setTexture(ETextureType::Normal, std::make_shared<MTexture>(TEXT("./Resources/Texture/Stone_01_normal.jpg")));
    Ground->GetStaticMeshCompoent()->setScale(20.f, 1.f, 20.f);
    Ground->GetStaticMeshCompoent()->setTranslation(1.f, -3.f, 0.f);
#endif


#if UseDirectionalLight == 1
    auto DirectionalLight = CreateActor<MDirectionalLightActor>(GetShared());
#endif

    auto Table = CreateActor<MStaticMeshActor>(GetShared());
    Table->GetStaticMeshCompoent()->SetMesh(TEXT("Table/Table.json"));
    Table->GetStaticMeshCompoent()->setScale(Vec3{ 0.02f, 0.02f, 0.02f });
    Table->GetStaticMeshCompoent()->setDrawingBoundingBox(true);
    Table->GetStaticMeshCompoent()->SetDrawCollision(true);

    auto a = CreateActor<MPointLightActor>(GetShared());
    a->GetPointLightComponent()->setRange(10.f);

    for (auto& [Name, Desc] : GetTypeDescs())
    {
        std::wcout << StringToWString(Desc->Name.c_str()) << std::endl;
    }
	return true;
}

void MyGame::Tick(const Time deltaTime)
{
    if (getRenderer() == nullptr)
    {
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello, world!");

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
        FHitData GizmoHitData = {};
        std::vector<FPrimitiveData> GizmoPrimitiveDatas;
        getRenderer()->GizmoMeshComp->GetPrimitiveData(GizmoPrimitiveDatas);

        if (Raycast(GizmoPrimitiveDatas, GizmoHitData))
        {
            if (bControlGizmo == false)
            {
                bSetGizmoOffset = true;
                bControlGizmo = true;
            }

            switch (GizmoHitData.PrimitiveIndex)
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
            if (Raycast(getRenderer()->GetRenderablePrimitiveData(), HitData))
            {
                ClickedComp = HitData.HitComponent;
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
            Vec2 Current = GetMousePos();
            ScreenToWorld(Current, 0.f, Near);
            ScreenToWorld(Current, 1.f, Far);

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
        HitData.HitComponent.reset();
        HitData.Distance = FLT_MAX;
    }
}

void MyGame::PostUpdate(const Time deltaTime)
{

}

void MyGame::render()
{
    if (getRenderer() == nullptr)
    {
        return;
    }

    if (ImGui::CollapsingHeader("Test Functions"))
    {
        if (ImGui::CollapsingHeader("Actor") && LanternActor)
        {
            auto IsNotEqual = [](float lhs, float rhs)->bool {
                return std::fabsf(lhs - rhs) > 0.00001;
            };

            ImGui::SliderFloat("ForceY", &Force, 0.f, 10000.f);
            if (ImGui::Button("AddForce"))
            {
                LanternActor->GetStaticMeshCompoent()->Temp(Force);
            }

            if (ImGui::Button("ResetVelocity"))
            {
                LanternActor->GetStaticMeshCompoent()->SetVelocity(0.f, 0.f, 0.f);
                LanternActor->GetStaticMeshCompoent()->SetAngularVelocity(0.f, 0.f, 0.f);
            }

            if (ImGui::Button("ResetPos"))
            {
                LanternActor->GetStaticMeshCompoent()->setTranslation(0.f, 5.f, 0.f);
            }
        }

        if (ImGui::CollapsingHeader("Level"))
        {
            ImGui::Indent(20);
            if (ImGui::Button("Save"))
            {
                MJsonSerializer Serializer;
                Serializer.Serialize(this, TEXT("D:\\Git\\Moon\\TestLevel.json"), true);
            }
            if (ImGui::Button("Load"))
            {
                GetLevelChangedDelegate().Broadcast();
                GetPostLoopDelegate().Add([]() {
                    std::unique_ptr<MainGame> NewGame = std::make_unique<MainGame>();
                    MJsonDeserializer Deserializer;
                    Deserializer.Deserialize(*NewGame.get(), TEXT("D:\\Git\\Moon\\TestLevel.json"));
                    setGame(std::move(NewGame));
                });

            }
            ImGui::Indent(-20);
        }

        if (ImGui::CollapsingHeader("JsonTest"))
        {
            ImGui::Indent(20);
            if (ImGui::Button("SaveJson"))
            {
                _pPlayer->JsonSaveTest();
            }
            if (ImGui::Button("SaveJsonPretty"))
            {
                _pPlayer->JsonSaveTest(true);
            }
            if (ImGui::Button("LoadJson"))
            {
                _pPlayer->JsonLoadTest();
            }
            ImGui::Indent(-20);
        }

        // FBX 로드 
        if (ImGui::CollapsingHeader("LoadFBX"))
        {
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
                    FBXLoader.SaveJsonAsset(FileName);
                    wcout << FileName << endl;
                }
            }
        }

        if (_pPlayer)
        {
            std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = std::static_pointer_cast<DynamicMeshComponent>(_pPlayer->getComponent(ROOT_COMPONENT));
            if (DynamicMeshComp && ImGui::Button("DynamicMeshCloth"))
            {
                DynamicMeshComp->Clothing();
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

    // 게임
    if (ImGui::CollapsingHeader("Game"))
    {
        if (ImGui::Button("Play"))
        {
            getMainGame()->PlayGame();
        }
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
        uint32 Num = GetSize(Actors);
        uint32 i = 0;
        for (auto& [Name, Actor] : Actors)
        {
            bool bHighlight = ClickedComp.expired() ? false : ClickedComp.lock()->getOwningActor() == Actor;

            if (bHighlight)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
            }

            if (ImGui::Selectable(Name.c_str()))
            {
                ClickedComp = Actor->getComponent(ROOT_COMPONENT);
            }

            if (bHighlight)
            {
                ImGui::PopStyleColor(1);
            }
        }
    }

    // 액터 편집 기능
    if (ImGui::CollapsingHeader("Actor Edit") && ClickedComp.expired() == false)
    {
        auto actor = ClickedComp.lock()->getOwningActor();
        for (auto& [Name, Comp] : actor->GetComponents())
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

    // 애셋 편집
    if (EditAsset)
    {
        std::string Name = EditAssetDesc->Name + " Edit";
        if(ImGui::Begin(Name.c_str()))
        { 
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Save"))
                {
                    MJsonSerializer Serializer;
                    Serializer.Serialize(EditAsset, EditAsset->GetAssetPath(), true);
                }
            }
            ImGui::EndMenu();

            const FTypeDesc* Current = EditAssetDesc;
            while (Current)
            {
                DispatchStruct(Current, EditAsset);
                Current = Current->Parent;
            }

            // 애셋 타입에 따라 추가 처리
            if (EditAsset->IsA<StaticMesh>())
            {
                //std::shared_ptr<StaticMesh> Mesh = nullptr;
                //g_ResourceManager->Load(EditAsset->GetAssetPath(), Mesh);
                if (ImGui::Button("Make Collision"))
                {
                    GetPhysics()->SaveTest(EditAsset->CastTo<StaticMesh>());
                }
            }
        }
        ImGui::End();
    }

	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool MyGame::IsPickable() const
{
    return ImGui::GetIO().WantCaptureMouse == false;
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
                if (InContainerDesc->bPointerElements)
                {
                    auto Asset = static_cast<MAsset*>(InContainerDesc->Get(InObject, i));
                    std::string Path = Asset == nullptr ? "" : WStringToString(Asset->GetAssetPath());

                    ImGui::Text(Path.c_str());

                    ImGui::SameLine(300);
                    if (ImGui::Button("Edit"))
                    {
                        GetGame<MyGame>()->EditAsset = Asset;
                        GetGame<MyGame>()->EditAssetDesc = InContainerDesc->TypeDesc;
                    }

                    ImGui::SameLine(350);
                    if (ImGui::Button("..."))
                    {
                        TCHAR FileName[256] = {};

                        OPENFILENAMEW t = {};
                        t.lStructSize = sizeof(t);
                        t.hwndOwner = NULL;
                        t.hInstance = NULL;
                        t.lpstrFilter = TEXT("json 파일\0*.fbx");
                        t.lpstrFile = FileName;
                        t.nMaxFile = 256;
                        t.lpstrInitialDir = TEXT(".");
                        t.lpstrTitle = TEXT("Load Asset");

                        if (GetOpenFileNameW(&t))
                        {
                            wcout << FileName << endl;
                        }
                    }

                    ImGui::NewLine();
                }
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
                MAsset* Asset = static_cast<MAsset*>(Prop->GetAsVoid(InObject));

                ImGui::Text(Prop->GetDisplayName().c_str());

                ImGui::SameLine(100);
                std::string Path = Asset == nullptr ? "Empty" : WStringToString(Asset->GetAssetPath());
                ImGui::Text(Path.c_str());
                    
                ImGui::SameLine(300);
                if (ImGui::Button("Edit"))
                {
                    static_cast<MyGame*>(getMainGame().get())->EditAsset = Asset;
                    static_cast<MyGame*>(getMainGame().get())->EditAssetDesc = Prop->TypeDesc;
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
                        std::shared_ptr<MAsset> NewAsset = g_ResourceManager->Load(FileName, Asset->GetTypeDesc());
                        static_cast<FFundamentalPropertyDesc<MAsset>*>(Prop)->Set(InObject, NewAsset);
                    }
                }

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
