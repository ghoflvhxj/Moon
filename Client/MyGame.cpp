#include "MyGame.h"
#include "MoonEngine.h"

#include "Core/ResourceManager.h"
#include "Core/Physics/Physics.h"
#include "Core/ObjectPath.h"
#include "Core/Asset.h"
#include "Core/Serialize/JsonSerializer.h"

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
	intializeImGui();
}

MyGame::~MyGame()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

const bool MyGame::initialize()
{
    MainGame::initialize();

	getMainCamera()->setLookMode(MCamera::LookMode::To);

	_pPlayer = CreateActor<Player>(this);

    LanternActor = CreateActor<MStaticMeshActor>(this);
    LanternActor->GetStaticMeshCompoent()->SetPhysicsType(EPhysicsType::Dynamic);
    LanternActor->SetStaticMesh(TEXT("Lantern/Lantern.fbx"));
    LanternActor->GetStaticMeshCompoent()->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
    LanternActor->GetStaticMeshCompoent()->SetDrawCollision(true);
    LanternActor->GetStaticMeshCompoent()->setDrawingBoundingBox(true);
    LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(false);
    LanternActor->GetStaticMeshCompoent()->RemovePhysics();

#if UseGround == 1
    auto Ground = CreateActor<MStaticMeshActor>(this);
    Ground->GetStaticMeshCompoent()->SetMesh(TEXT("Base/Box.json"));
    Ground->GetStaticMeshCompoent()->GetMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, std::make_shared<MTexture>(TEXT("./Resources/Texture/stone_01_albedo.jpg")));
    Ground->GetStaticMeshCompoent()->GetMesh()->getMaterial(0)->setTexture(ETextureType::Normal, std::make_shared<MTexture>(TEXT("./Resources/Texture/Stone_01_normal.jpg")));
    Ground->GetStaticMeshCompoent()->setScale(20.f, 1.f, 20.f);
    Ground->GetStaticMeshCompoent()->setTranslation(1.f, -3.f, 0.f);
#endif


#if UseDirectionalLight == 1
    auto DirectionalLight = CreateActor<MDirectionalLightActor>(this);
#endif

    auto Table = CreateActor<MStaticMeshActor>(this);
    Table->GetStaticMeshCompoent()->SetMesh(TEXT("Table/Table.json"));
    Table->GetStaticMeshCompoent()->setScale(Vec3{ 0.02f, 0.02f, 0.02f });
    Table->GetStaticMeshCompoent()->setDrawingBoundingBox(true);
    Table->GetStaticMeshCompoent()->SetDrawCollision(true);

    auto a = CreateActor<MPointLightActor>(this);

    std::filesystem::path CurrentPath = std::filesystem::current_path();
    wcout << CurrentPath.wstring() << endl;

	return true;
}

void MyGame::intializeImGui()
{
	// ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.Fonts->AddFontFromFileTTF("Resources/Fonts/NanumSquareRoundR.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(getGraphicDevice()->getDevice(), getGraphicDevice()->getContext());

    //io.WantCaptureKeyboard = true;
}

void MyGame::Tick(const Time deltaTime)
{
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
    std::shared_ptr<MLightComponent> DirectionalLight = std::static_pointer_cast<MLightComponent>(_pPlayer->getComponent(TEXT("DirectionalLight")));
    std::shared_ptr<MLightComponent> PointLight = std::static_pointer_cast<MLightComponent>(_pPlayer->getComponent(TEXT("PointLight")));
    std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = std::static_pointer_cast<DynamicMeshComponent>(_pPlayer->getComponent(ROOT_COMPONENT));
    
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

        if (DynamicMeshComp && ImGui::Button("DynamicMeshCloth"))
        {
            DynamicMeshComp->Clothing();
        }
    }

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
        uint32 Num = GetSize(_actorList);
        uint32 i = 0;
        for (auto actor : _actorList)
        {
            std::string name = "Actor_" + std::to_string(i++) + "(" + actor->GetTypeDesc()->Name + ")";

            bool bHighlight = ClickedComp.expired() ? false : ClickedComp.lock()->getOwningActor() == actor;

            if (bHighlight)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
            }

            if (ImGui::Selectable(name.c_str()))
            {
                ClickedComp = actor->getComponent(ROOT_COMPONENT);
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

    // 메시 편집 기능. 일단 임시로 컴포넌트에서 메시를 가져옴
    if (std::shared_ptr<MMeshComponent> HitComponent = std::static_pointer_cast<MMeshComponent>(HitData.HitComponent.lock()))
    {
        if (auto TestMesh = HitComponent->GetMesh())
        {
            if (ImGui::CollapsingHeader("Mesh Edit"))
            {
                const FTypeDesc* Current = TestMesh->GetTypeDesc();
                while (Current)
                {
                    DispatchStruct(Current, TestMesh.get());
                    Current = Current->Parent;
                }
            }
        }
    }

    // 매터리얼 에디트
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
                ImGui::EndMenu();
            }

            const FTypeDesc* Current = EditAssetDesc;
            while (Current)
            {
                DispatchStruct(Current, EditAsset);
                Current = Current->Parent;
            }

            ImGui::End();
        }
    }

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool MyGame::IsPickable() const
{
    return ImGui::IsWindowHovered() == false;
}

void DispatchContainer(const FTypeDesc* InElementTypeDesc, FContainerPropertyDesc* InContainerDesc, void* InObject)
{
    if (InObject == nullptr)
    {
        return;
    }

    uint32 ElementNum = InContainerDesc->GetNum(InObject);

    if (InContainerDesc->IsA<MAsset>() && InContainerDesc->bPointerElements)
    {
        if (ImGui::CollapsingHeader(InContainerDesc->Name.c_str()))
        {
            for (uint32 i = 0; i < ElementNum; ++i)
            {
                auto Asset = static_cast<MAsset*>(InContainerDesc->Get(InObject, i));
                std::string Path = Asset == nullptr ? "" : WStringToString(Asset->GetAssetPath());

                ImGui::Text(Path.c_str());

                ImGui::SameLine(300);
                if (ImGui::Button("Edit"))
                {
                    static_cast<MyGame*>(getMainGame().get())->EditAsset = Asset;
                    static_cast<MyGame*>(getMainGame().get())->EditAssetDesc = InContainerDesc->TypeDesc;
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
                    t.lpstrTitle = TEXT("Load FBX");

                    if (GetOpenFileNameW(&t))
                    {
                        wcout << FileName << endl;
                    }
                }

                ImGui::NewLine();
            }
        }
    }
    else
    {
        for (int i = 0; i < ElementNum; ++i)
        {
            if (InElementTypeDesc)
            {
                for (auto& Prop : InElementTypeDesc->Properties)
                {
                    switch (Prop->Type)
                    {
                    case EType::Int:
                    {
                        int& Temp = static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InContainerDesc->Get(InObject, i));
                    }
                    break;
                    case EType::Float:
                    {
                        float& Temp = static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InContainerDesc->Get(InObject, i));
                    }
                    break;
                    case EType::Vec2:
                    case EType::Vec3:
                    case EType::Vec4:
                    {

                    }
                    break;
                    default:
                    {
                        size_t num = InContainerDesc->GetNum(InObject);
                        if (Prop->bContainer)
                        {
                            DispatchContainer(Prop->TypeDesc, static_cast<FContainerPropertyDesc*>(Prop), InContainerDesc->Get(InObject, i));
                        }
                        else
                        {
                            DispatchStruct(Prop->TypeDesc, InContainerDesc->Get(InObject, i));
                        }
                    }
                    break;
                    }
                }
            }
            else
            {
                switch (InContainerDesc->Type)
                {
                case EType::Int:
                case EType::Enum:
                {
                    int& Temp = *(int*)InContainerDesc->Get(InObject, i);
                    ImGui::InputInt((InContainerDesc->Name + std::to_string(i)).c_str(), &Temp);
                }
                break;
                case EType::Float:
                {
                    float& Temp = *(float*)InContainerDesc->Get(InObject, i);
                    ImGui::InputFloat((InContainerDesc->Name + std::to_string(i)).c_str(), &Temp);
                }
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
        if (Prop->bContainer)
        {
            DispatchContainer(Prop->TypeDesc, static_cast<FContainerPropertyDesc*>(Prop), InObject);
        }
        else if (Prop->Num > 1) // 배열
        {
            uint32 Num = Prop->Num;
            if (ImGui::CollapsingHeader(Prop->Name.c_str()))
            {
                for (uint32 i = 0; i < Num; ++i)
                {
                    std::string NameString = Prop->Name + std::to_string(i);
                    const char* Name = NameString.c_str();
                    switch (Prop->Type)
                    {
                        case EType::Int:
                        case EType::Enum:
                        {
                            int& Temp = static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InObject, i);
                            ImGui::InputInt(Name, &Temp);
                        }
                        break;
                        case EType::Float:
                        {
                            float& Temp = static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InObject, i);
                            ImGui::InputFloat(Name, &Temp);
                        }
                        break;
                        case EType::Bool:
                        {
                            bool& Temp = static_cast<FFundamentalPropertyDesc<bool>*>(Prop)->Get(InObject, i);
                            ImGui::Checkbox(Name, &Temp);
                        }
                        case EType::Vec2:
                        case EType::Vec4:
                        {

                        }
                        break;
                        case EType::Vec3:
                        {
                            auto& Temp = static_cast<FFundamentalPropertyDesc<Vec3>*>(Prop)->Get(InObject, i);
                            float TempArr[3] = { Temp.x, Temp.y, Temp.z };
                            if (ImGui::InputFloat3(Name, TempArr))
                            {
                                Temp = { TempArr[0], TempArr[1], TempArr[2] };
                            }
                        }
                        break;
                        case EType::WString:
                        {
                            auto& Temp = static_cast<FFundamentalPropertyDesc<std::wstring>*>(Prop)->Get(InObject, i);
                            char Buff[256] = {};
                            WStringToString(Temp, Buff, 256);

                            if (ImGui::InputText(Name, Buff, 256))
                            {

                            }
                        }
                        break;
                        default:
                        {
                            uint64 Base = (uint64)Prop->GetAsVoid(InObject);
                            uint64 MemoryPos = Base + (Prop->GetSize() * i);
                            DispatchStruct(Prop->TypeDesc, (void*)MemoryPos);
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            switch (Prop->Type)
            {
            case EType::Int:
            case EType::Enum:
            {
                int& Temp = static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InObject);
                ImGui::InputInt(Prop->Name.c_str(), &Temp);
            }
            break;
            case EType::Float:
            {
                float& Temp = static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InObject);
                ImGui::InputFloat(Prop->Name.c_str(), &Temp);
            }
            break;
            case EType::Bool:
            {
                bool& Temp = static_cast<FFundamentalPropertyDesc<bool>*>(Prop)->Get(InObject);
                ImGui::Checkbox(Prop->Name.c_str(), &Temp);
            }
            break;
            case EType::Vec2:
            case EType::Vec4:
            {

            }
            break;
            case EType::Vec3:
            {
                auto& Temp = static_cast<FFundamentalPropertyDesc<Vec3>*>(Prop)->Get(InObject);
                float TempArr[3] = { Temp.x, Temp.y, Temp.z };
                if (ImGui::InputFloat3(Prop->Name.c_str(), TempArr))
                {
                    Temp = { TempArr[0], TempArr[1], TempArr[2] };
                }
            }
            break;
            case EType::WString:
            {
                auto& Temp = static_cast<FFundamentalPropertyDesc<std::wstring>*>(Prop)->Get(InObject);
                char Buff[256] = {};
                WStringToString(Temp, Buff, 256);

                if (ImGui::InputText(Prop->Name.c_str(), Buff, 256))
                {

                }
            }
            break;
            default:
            {
                if (Prop->TypeDesc && Prop->IsA<MAsset>())
                {
                    MAsset* Asset = static_cast<MAsset*>(Prop->GetAsVoid(InObject));
                    std::string Path = Asset == nullptr ? "" : WStringToString(Asset->GetAssetPath());

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
            break;
            }
        }
    }
}