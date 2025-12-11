#include "ActorEditor.h"
#include "MoonEngine.h"

#include "imgui.h"

MActorEditor::MActorEditor()
{
    Title = "New Actor";
}

void MActorEditor::Update()
{
    Working->update(0.f);
}

void MActorEditor::HandleObject()
{
    Working = WorkingObject->CastToShared<MActor>();
    assert(Working);

    W->addActor(Working);
}

void MActorEditor::OnSaved()
{
    assert(SourceObject);
    SourceObject->CastToShared<MActor>()->update(0.f);
}

void MActorEditor::RenderUI()
{
    MEditorBase::RenderUI();

    //if (ImGui::Begin("t1", &bOpen))
    //{
    //    for (auto& [Name, Comp] : Working->GetComponents())
    //    {
    //        if (ImGui::CollapsingHeader(WStringToString(Name).c_str()))
    //        {
    //            // 컴포넌트 속성 편집 기능
    //            const FTypeDesc* Current = Comp->GetTypeDesc();
    //            while (Current)
    //            {
    //                ImGui::Indent(20.f);
    //                if (ImGui::CollapsingHeader(Current->Name.c_str()))
    //                {
    //                    DispatchStruct(Current, Comp.get());
    //                }
    //                ImGui::Indent(-20.f);

    //                Current = Current->Parent;
    //            }
    //        }
    //    }

    //    ImGui::End();
    //}
}
