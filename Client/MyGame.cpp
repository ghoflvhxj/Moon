#include "MyGame.h"
#include "MoonEngine.h"

#include "GraphicDevice.h"

#include "Renderer.h"

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
#include "Core/ResourceManager.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Material.h"
#include "Core/Physics/Physics.h"

#include "GameFramework/StaticMeshActor/StaticMeshActor.h"
#include "GameFramework/PointLightActor/PointLightActor.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

using namespace DirectX;

MyGame::MyGame()
	: MainGame()
	, _pTerrainComponent{ nullptr }
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

    //LanternActor = CreateActor<MStaticMeshActor>(this);
    //LanternActor->GetStaticMeshCompoent()->SetPhysicsType(EPhysicsType::Dynamic);
    //LanternActor->SetStaticMesh(TEXT("Lantern/Lantern.fbx"));
    //LanternActor->GetStaticMeshCompoent()->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
    //LanternActor->GetStaticMeshCompoent()->SetDrawCollision(true);
    //LanternActor->GetStaticMeshCompoent()->setDrawingBoundingBox(true);
    //LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(false);
    //LanternActor->GetStaticMeshCompoent()->RemovePhysics();

    //ClothActor = CreateActor<MStaticMeshActor>(this);
    //ClothActor->GetStaticMeshCompoent()->SetPhysics(false);
    //ClothActor->SetStaticMesh(TEXT("Untitled.fbx"));
    //ClothActor->GetStaticMeshCompoent()->setTranslation(0.f, 6.f, 0.f);

    //ClothActor->GetStaticMeshCompoent()->GetMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, std::make_shared<MTexture>(TEXT("./Resources/Texture/stone_01_albedo.jpg")));
    //ClothActor->GetStaticMeshCompoent()->GetMesh()->getMaterial(0)->setTexture(ETextureType::Normal, std::make_shared<MTexture>(TEXT("./Resources/Texture/Stone_01_normal.jpg")));

    //std::shared_ptr<MTexture> Texture = nullptr;
    //if (g_ResourceManager->Load<MTexture>(TEXT("Resources/Texture/Player.jpeg"), Texture))
    //{
    //    ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, Texture);
    //    ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setCullMode(Graphic::CullMode::None);
    //    //ClothActor->GetStaticMeshCompoent()->getStaticMesh()->getMaterial(0)->setFillMode(Graphic::FillMode::WireFrame);
    //}

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

    if (InputManager::mouseDown(MOUSEBUTTON::LB) && IsPickable())
    {
        Pick();
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
    std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = std::static_pointer_cast<DynamicMeshComponent>(_pPlayer->getComponent(TEXT("DynamicMesh")));
    
	ImGui::Text("Toatal primitive:%d", getRenderer()->TotalPrimitiveNum);
	ImGui::Text("show primitive:%d", getRenderer()->ShownPrimitiveNum);
	ImGui::Text("culled primitive:%d", getRenderer()->CulledPrimitiveNum);
	ImGui::Checkbox("Debug Collision", &getRenderer()->bDrawCollision);

	if (ImGui::CollapsingHeader("DirectionalLight") && DirectionalLight)
	{
		Vec3 rot = DirectionalLight->getRotation();
		ImGui::SliderAngle("rotX", &rot.x);
		ImGui::SliderAngle("rotY", &rot.y);
		ImGui::SliderAngle("rotZ", &rot.z);
        DirectionalLight->setRotation(rot);
	}

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

        //if (ImGui::Checkbox("Physics Simulation", &bStaticCollision))
        //{
        //    LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(bStaticCollision);
        //}
        LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(false);
	}
    else
    {
        //LanternActor->GetStaticMeshCompoent()->SetPhysicsSimulate(true);
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

    // 바디와 옷 충돌 테스트
    if (DynamicMeshComp->BodyTestObject)
    {
        Vec3 a = DynamicMeshComp->BodyTestObject->GetPhysicsPos();
        LanternActor->GetStaticMeshCompoent()->setTranslation(a);
    }

    // 조인트 위치 테스트
    //if (LanternActor && DynamicMeshComp)
    //{
    //    LanternActor->GetStaticMeshCompoent()->setTranslation(DynamicMeshComp->GetJointPosition("bone001"));
    //}

    //Vec3 JointPos = DynamicMeshComp->GetJointPosition("bone001");
    //std::cout << "JointPos X: " << JointPos.x << ", Y: " << JointPos.y << ", Z: " << JointPos.z << std::endl;

    if (ClothActor && ImGui::Button("Cloth"))
    {
        ClothActor->GetStaticMeshCompoent()->Clothing();
    }

    if (DynamicMeshComp && DynamicMeshComp->PhysicsObject && ImGui::Button("DynamicMeshCloth Pos"))
    {
        DynamicMeshComp->PhysicsObject->SetPos(DynamicMeshComp->GetJointPosition("bone014"));
    }

    if(DynamicMeshComp && ImGui::Button("DynamicMeshCloth"))
    {
        DynamicMeshComp->Clothing();
        FMeshData a;
        static_cast<FContainerPropertyDesc*>(a.GetTypeDesc()->Properties[0])->Resize(&a, 10000);
        static_cast<FContainerPropertyDesc*>(a.GetTypeDesc()->Properties[1])->Resize(&a, 10000);
    }

    if (std::shared_ptr<Component> HitComponent = HitData.HitComponent.lock())
    {
        const FTypeDesc* Current = HitComponent->GetTypeDesc();
        while (Current)
        {
            if (ImGui::CollapsingHeader(Current->Name.c_str()))
            {
                DispatchStruct(Current, HitComponent.get());
            }
            //for (auto Prop : Current->Properties)
            //{
            //    if (Prop->bContainer)
            //    {
            //        DispatchContainer(Prop->TypeDesc, static_cast<FContainerPropertyDesc*>(Prop), HitComponent.get());
            //    }
            //    else
            //    {
            //        
            //    }
            //}

            Current = Current->Parent;
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
    // 벡터 요소들을 순회하면서
    for (int i = 0; i < InContainerDesc->GetNum(InObject); ++i)
    {
        // 요소들 타입에 따라 출력
        if (InElementTypeDesc) // EngineDataType
        {
            for (auto& Prop : InElementTypeDesc->Properties)
            {
                //cout << Prop->Name + ": ";
                switch (Prop->Type)
                {
                case EType::Int:
                {
                    int& Temp = static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InContainerDesc->Get(InObject, i));
                    //cout << Temp << endl;
                }
                break;
                case EType::Float:
                {
                    float& Temp = static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InContainerDesc->Get(InObject, i));
                    //cout << Temp << endl;
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
        else // FundamentalDataType
        {
            switch (InContainerDesc->Type)
            {
            case EType::Int:
            {
                int& Temp = *(int*)InContainerDesc->Get(InObject, i);
                //cout << Temp << endl;
            }
            break;
            case EType::Float:
            {
                float& Temp = *(float*)InContainerDesc->Get(InObject, i);
                //cout << Temp << endl;
            }
            break;
            }
        }

    }
}

void DispatchStruct(const FTypeDesc* InStructDesc, void* InObject)
{
    for (auto& Prop : InStructDesc->Properties)
    {
        switch (Prop->Type)
        {
        case EType::Int:
        {
            int& Temp = static_cast<FFundamentalPropertyDesc<int>*>(Prop)->Get(InObject);
            //cout << Temp << endl;
        }
        break;
        case EType::Float:
        {
            float& Temp = static_cast<FFundamentalPropertyDesc<float>*>(Prop)->Get(InObject);
            //cout << Temp << endl;
        }
        break;
        case EType::Bool:
        {
            bool &Temp = static_cast<FFundamentalPropertyDesc<bool>*>(Prop)->Get(InObject);
            ImGui::Checkbox(Prop->Name.c_str(), &Temp);
        }
        break;
        case EType::Vec3:
        {
            auto &Temp = static_cast<FFundamentalPropertyDesc<Vec3>*>(Prop)->Get(InObject);
            float TempArr[3] = { Temp.x, Temp.y, Temp.z };
            if (ImGui::InputFloat3(Prop->Name.c_str(), TempArr))
            {
                Temp = { TempArr[0], TempArr[1], TempArr[2] };
            }
        }
        break;
        default:
        {
            if (Prop->bContainer)
            {
                DispatchContainer(Prop->TypeDesc, static_cast<FContainerPropertyDesc*>(Prop), InObject);
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