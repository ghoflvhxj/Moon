#include "MainWindow.h"
#include "MoonEngine.h"

#include "Editor.h"
#include "Renderer.h"

#include "WindowManager.h"

#include "World.h"
#include "SceneComponent.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"
#include "Camera.h"

#include "Gameframework/StaticmeshActor/StaticMeshActor.h"
#include "Core/ResourceManager.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

// FBX
#include "FBXLoader.h"

const ImVec4 HighlightColor = { 1.f, 1.f, 0.f, 1.f };

MEditorMainWindow::MEditorMainWindow(const std::wstring& title, const int width, const int height, const std::wstring& className)
    : Super(title, width, height, className)
{
}

MEditorMainWindow::MEditorMainWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className)
    : Super(title, width, height, Parent, className)
{
}

void MEditorMainWindow::ImGuiRender()
{
    const auto& EditorModule = GetEngine()->GetModule<MEditor>();
    const auto& ClickedComp = EditorModule->GetClickedComp();

    if (ImGui::Begin("World"))
    {
        if (ImGui::CollapsingHeader("Test Functions"))
        {
            auto SelectedComp = ClickedComp == nullptr ? nullptr : ClickedComp->CastTo<StaticMeshComponent>();
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
                ImGui::PushID("LoadFBX");

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

                ImGui::PopID();
            }

            if (ImGui::CollapsingHeader("Jolt Physics"))
            {
                if (ImGui::Button("Jolt Save") && ClickedComp)
                {
                    if (std::shared_ptr<MMeshComponent> MeshComp = ClickedComp->CastToShared<MMeshComponent>())
                    {
                        if (MeshComp->GetMesh())
                        {
                            GetPhysics()->SaveTest(MeshComp->GetMesh());
                        }
                    }
                }

                if (ImGui::Button("Jolt Load") && ClickedComp)
                {
                    if (std::shared_ptr<MMeshComponent> MeshComp = ClickedComp->CastToShared<MMeshComponent>())
                    {
                        if (MeshComp->GetMesh())
                        {
                            GetPhysics()->LoadTest();
                        }
                    }
                }

                if (ImGui::Button("Clothing2") && ClickedComp)
                {
                    if (auto DynamicMeshComp = ClickedComp->CastToShared<DynamicMeshComponent>())
                    {
                        DynamicMeshComp->Clothing2();
                    }
                }
            }
        }

        if (ImGui::CollapsingHeader("Level"))
        {
            ImGui::PushID("Level");

            ImGui::Indent(20);
            if (ImGui::Button("Save As"))
            {
                EditorModule->SaveAs(*GetMainWorld());
            }
            if (ImGui::Button("Load"))
            {
                GetLevelChangedDelegate().Broadcast();

                EditorModule->Open([&](const TCHAR* InFileName) {
                    std::wstring FileName = InFileName;
                    GetPostLoopDelegate().Add([FileName]() {
                        GetMainWorld()->GetActors().clear();
                        GetMainWorld()->Load(FileName);
                        });
                    });
            }
            ImGui::Indent(-20);

            ImGui::PopID();
        }

        // 게임
        if (ImGui::CollapsingHeader("Game"))
        {
            ImGui::Indent(20);
            if (ImGui::Button("Play"))
            {
                std::shared_ptr<MWindow> NewWindow = GetWindowManager()->CreateWindow<MWindow>(TEXT("PIE"), GetMainWindow()->GetWidth<int>(), GetMainWindow()->GetHeight<int>(), g_hWnd, TEXT("ShootingGame"));
                NewWindow->SetWindowPos(GetMainWindow()->GetWindowPos());
                NewWindow->Initialize();

                //std::shared_ptr<MWorld> NewWorld = DuplicateObject(GetMainWorld())->CastToShared<MWorld>();
                std::shared_ptr<MWorld> NewWorld = std::make_shared<MWorld>();
                NewWorld->SetWorldType(EWorldType::PIayInEditor);
                NewWorld->Initialize();
                NewWorld->getMainCamera()->SetWorldTranslation({ 0.f, 0.f, -2.f });

                GetEngine()->AddWorld(NewWorld, NewWindow);

                NewWorld->DuplicateActors(GetMainWorld());
                NewWorld->PlayGame();
            }

            ImGui::Indent(-20);
        }

        // 기즈모 컨트롤
        for (int i = 0; i < (int)EGizmoMode::Count; ++i)
        {
            bool bHighlight = (int)EditorModule->GetGizmoMode() == i;

            if (bHighlight)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, HighlightColor);
            }

            std::array<std::string, 3> Names = { "Translation", "Rotation", "Scale" };

            if (ImGui::Button(Names[i].c_str()))
            {
                EditorModule->SetGizmoMode((EGizmoMode)i);
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
                DispatchType(Current, getRenderer().get());
                Current = Current->Parent;
            }
        }

        // 하이어라키
        if (ImGui::CollapsingHeader("Hierarchy"))
        {
            auto& Actors = GetMainWorld()->GetActors();
            uint32 Num = GetSize(Actors);

            std::shared_ptr<MActor> HighlightActor = ClickedComp == nullptr ? nullptr : ClickedComp->getOwningActor();

            // 하이라이트, Tick에서 계속 순회하는 것 보다는 변경 시 업데이트 해주는게 나을듯?
            for (auto& [Name, Actor] : Actors)
            {
                bool bHighlight = HighlightActor == Actor;
                if (bHighlight)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                }

                if (ImGui::Selectable(Name.c_str()))
                {
                    EditorModule->SetClickedComp(Actor->getComponent(ROOT_COMPONENT));
                }

                if (bHighlight)
                {
                    ImGui::PopStyleColor(1);
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
                EditorModule->SetClickedComp(NewActor->getComponent(ROOT_COMPONENT));
            }
        }

        // 액터 편집 기능
        if (ClickedComp)
        {
            if (std::shared_ptr<MActor> Actor = ClickedComp->getOwningActor())
            {
                if (ImGui::CollapsingHeader("Actor Edit"))
                {
                    DispatchType2(Actor->GetTypeDesc(), Actor.get());

                    //for (auto& [Name, Comp] : Actor->GetComponents())
                    //{
                    //    char CName[128];
                    //    WStringToString(Name, CName, 128);
                    //    std::string ClassName = "(" + Comp->GetTypeDesc()->Name + ")";
                    //    strcat_s(CName, 128, ClassName.c_str());

                    //    bool bHighlight = ClickedComp == Comp;
                    //    if (bHighlight)
                    //    {
                    //        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                    //    }

                    //    if (ImGui::Selectable(CName))
                    //    {
                    //        EditorModule->SetClickedComp(Comp);
                    //    }

                    //    if (bHighlight)
                    //    {
                    //        ImGui::PopStyleColor(1);
                    //    }
                    //}

                    //// 컴포넌트 속성 편집 기능
                    //const FTypeDesc* Current = ClickedComp->GetTypeDesc();
                    //while (Current)
                    //{
                    //    ImGui::Indent(20.f);
                    //    if (ImGui::CollapsingHeader(Current->Name.c_str()))
                    //    {
                    //        DispatchType(Current, ClickedComp.get());
                    //    }
                    //    ImGui::Indent(-20.f);

                    //    Current = Current->Parent;
                    //}
                }

                if (ImGui::BeginPopupContextVoid("Test", ImGuiPopupFlags_MouseButtonRight))
                {
                    if (ImGui::MenuItem("Add Capsule"))
                    {
                        EditorModule->SaveAs<MActor>(*Actor);
                    }

                    if (ImGui::MenuItem("Delete"))
                    {
                        GetMainWorld()->RemoveActor(Actor.get());
                    }

                    if (ImGui::MenuItem("Edit"))
                    {
                        OpenEditor(Actor);
                    }

                    ImGui::SetCurrentContext(Context);
                    ImGui::EndPopup();
                }

                Actor->update(0.f);
            }
        }

        ImGui::End();
    }

    //if (ImGui::Begin("Browser"))
    //{


    //    ImGui::End();
    //}
}

MEditorBaseWindow::~MEditorBaseWindow()
{
    if (Context != nullptr)
    {
        ImGui::SetCurrentContext(Context);

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext(Context);

        Context = nullptr;
    }
}

MEditorBaseWindow::MEditorBaseWindow(const std::wstring& title, const int width, const int height, const std::wstring& className)
    : Super(title, width, height, className)
{
}

MEditorBaseWindow::MEditorBaseWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className)
    : Super(title, width, height, className)
{
}

void MEditorBaseWindow::Initialize()
{
    InitImGui();
}

void MEditorBaseWindow::Render()
{
    Super::Render();

    if (SetImGuiContext() == false)
    {
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiRender();
    GetOnImGuiRenderedDelegate().Broadcast();

    ImGui::Render();
    ImGui::EndFrame();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void MEditorBaseWindow::ImGuiRender()
{
    
}

void MEditorBaseWindow::InitImGui()
{
    if (getGraphicDevice() == nullptr)
    {
        return;
    }

    Context = ImGui::CreateContext();
    ImGui::SetCurrentContext(Context);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    std::wstring WFontPath = MFIleSystem::AbsolutePath("Resources/Fonts/NanumSquareRoundR.ttf");
    std::string FontPath = WStringToString(WFontPath);
    io.Fonts->AddFontFromFileTTF(FontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(getHandle());
    ImGui_ImplDX11_Init(getGraphicDevice()->getDevice(), getGraphicDevice()->getContext());
}

bool MEditorBaseWindow::SetImGuiContext()
{
    if (Context != nullptr)
    {
        ImGui::SetCurrentContext(Context);
        return true;
    }

    return false;
}
