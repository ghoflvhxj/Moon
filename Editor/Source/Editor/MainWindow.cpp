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

#include "Module/Render/Scene.h"

#include "Framework/StaticmeshActor/StaticMeshActor.h"
#include "Core/ResourceManager.h"
#include "Core/FileSystem.h"

#include "imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

// FBX
#include "FBXLoader.h"

const ImVec4 HighlightColor = { 1.f, 1.f, 0.f, 1.f };

MEditorMainWindow::MEditorMainWindow()
    : Super()
{
}

void MEditorMainWindow::ImGuiRender()
{
    const auto& EditorModule = GetEngine()->GetModule<MEditor>();
    const auto& ClickedComp = EditorModule->GetClickedComp();

    if (ImGui::Begin("World"))
    {
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
    }
    ImGui::End();

    if (ImGui::Begin("Actor Edit") && ClickedComp)
    {
        // 액터 편집 기능
        if (std::shared_ptr<MActor> Actor = ClickedComp->getOwningActor())
        {
            //DispatchType2(Actor->GetTypeDesc(), Actor.get());

            for (auto& [Name, Comp] : Actor->GetComponents())
            {
                char CName[128];
                WStringToString(Name, CName, 128);
                std::string ClassName = "(" + Comp->GetTypeDesc()->Name + ")";
                strcat_s(CName, 128, ClassName.c_str());

                bool bHighlight = ClickedComp == Comp;
                if (bHighlight)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
                }

                if (ImGui::Selectable(CName))
                {
                    EditorModule->SetClickedComp(Comp);
                }

                if (bHighlight)
                {
                    ImGui::PopStyleColor(1);
                }
            }

            // 컴포넌트 속성 편집 기능
            const FTypeDesc* Current = ClickedComp->GetTypeDesc();
            while (Current)
            {
                ImGui::Indent(20.f);
                if (ImGui::CollapsingHeader(Current->Name.c_str()))
                {
                    DispatchType(Current, ClickedComp.get());
                }
                ImGui::Indent(-20.f);

                Current = Current->Parent;
            }

            Actor->update(0.f);

            //if (ImGui::BeginPopupContextVoid("Test", ImGuiPopupFlags_MouseButtonRight))
            //{
            //    if (ImGui::MenuItem("Add Capsule"))
            //    {
            //        EditorModule->SaveAs<MActor>(*Actor);
            //    }

            //    if (ImGui::MenuItem("Delete"))
            //    {
            //        GetMainWorld()->RemoveActor(Actor.get());
            //    }

            //    if (ImGui::MenuItem("Edit"))
            //    {
            //        OpenEditor(Actor);
            //    }

            //    ImGui::SetCurrentContext(Context);
            //    ImGui::EndPopup();
            //}
        }

    }
    ImGui::End();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Level"))
        {
            if (ImGui::MenuItem("Save"))
            {

            }
            if (ImGui::MenuItem("Save As"))
            {
                EditorModule->SaveAs(*GetMainWorld(), TEXT("Level File(*.level)\0*.level\0\0"));
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Load"))
            {
                EditorModule->Open([&](const TCHAR* InPathStr) {
                    std::wstring Path = InPathStr;
                    if (MFileSystem::IsExist(InPathStr))
                    {
                        GetEngine()->OpenLevel(Path);
                        SetTitle(Path);
                    }
                    }, TEXT("Level File(*.level)\0*.level\0\0"));
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Editor"))
        {
            if (ImGui::BeginMenu("Gizmo"))
            {
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

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tool"))
        {
            if (ImGui::BeginMenu("Load Mesh"))
            {
                static bool bMesh = false;
                static bool bMaterial = false;
                static bool bSkeleton = false;
                static bool bAnim = false;

                //ImGui::MenuItem("Mesh", nullptr, &bMesh);
                //ImGui::MenuItem("Material", nullptr, &bMaterial);
                //ImGui::MenuItem("Skeleton", nullptr, &bSkeleton);
                //ImGui::MenuItem("Anim", nullptr, &bAnim);

                ImGui::Text("-Option-");
                ImGui::Checkbox("Mesh", &bMesh);
                ImGui::Checkbox("Material", &bMaterial);
                ImGui::Checkbox("Skeleton", &bSkeleton);
                ImGui::Checkbox("Anim", &bAnim);

                ImGui::Separator();

                if (ImGui::MenuItem("Load"))
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
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render"))
        {
            // 렌더러
            //if (ImGui::MenuItem("Show Info"))
            {
                auto Renderer = getRenderer();
                ImGui::Text("Toatal primitive:%d", Renderer->TotalPrimitiveNum);
                ImGui::Text("show primitive:%d", Renderer->ShownPrimitiveNum);
                ImGui::Text("culled primitive:%d", Renderer->CulledPrimitiveNum);
                ImGui::Text("Frame: %d", GetMainWorld()->getFrame());

                DispatchType2(Renderer->GetTypeDesc(), Renderer.get());
                DispatchType2(Renderer->GetScene(0)->GetTypeDesc(), Renderer->GetScene(0));

                if (ImGui::CollapsingHeader("Performance"))
                {
                    ImGui::Text(WStringToString(GetEngine()->CPUTime).c_str());
                    for (auto PassTime : getGraphicDevice()->RenderPassTimes)
                    {
                        ImGui::Text(WStringToString(PassTime).c_str());
                    }
                }
            }

            ImGui::EndMenu();
        }

        // 게임
        if (ImGui::BeginMenu("Game"))
        {
            if (ImGui::MenuItem("Play"))
            {
                std::shared_ptr<MWindow> NewWindow = GetWindowManager()->AddWindow<MWindow>(TEXT("PIE"), GetMainWindow()->GetWidth<int>(), GetMainWindow()->GetHeight<int>(), NULL, TEXT("ShootingGame"));
                NewWindow->SetWindowPos(GetMainWindow()->GetWindowPos());

                //std::shared_ptr<MWorld> NewWorld = DuplicateObject(GetMainWorld())->CastToShared<MWorld>();
                std::shared_ptr<MWorld> NewWorld = std::make_shared<MWorld>();
                NewWorld->SetWorldType(EWorldType::PIayInEditor);

                GetEngine()->AddWorld(NewWorld, NewWindow);

                NewWorld->DuplicateActors(GetMainWorld());
                NewWorld->PlayGame();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    //ImGui::End();
}

void MEditorMainWindow::Copy()
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    EditorModule->SetCopyActor();
}

void MEditorMainWindow::Paste()
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    EditorModule->CreateCopyActor();
}

MEditorBaseWindow::MEditorBaseWindow()
{
    EditorModule = GetEngine()->GetModule<MEditor>().get();
    assert(EditorModule);
}

MEditorBaseWindow::~MEditorBaseWindow()
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

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGuiRender();
    GetOnImGuiRenderedDelegate().Broadcast();

    ImGui::Render();
    ImGui::EndFrame();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void MEditorBaseWindow::Release()
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

void MEditorBaseWindow::ImGuiRender()
{
    
}

void MEditorBaseWindow::InitImGui()
{
    if (getGraphicDevice())
    {
        Context = ImGui::CreateContext();
        ImGui::SetCurrentContext(Context);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
        std::wstring WFontPath = MFileSystem::AbsolutePath("Resources/Fonts/NanumSquareRoundR.ttf");
        std::string FontPath = WStringToString(WFontPath);
        io.Fonts->AddFontFromFileTTF(FontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(getHandle());
        ImGui_ImplDX11_Init(getGraphicDevice()->getDevice(), getGraphicDevice()->getContext());
    }
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
