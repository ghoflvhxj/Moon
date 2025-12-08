#include "ActorEditor.h"
#include "MoonEngine.h"

#include "imgui.h"

MActorEditor::MActorEditor()
{

}

void MActorEditor::SetObject(std::shared_ptr<MObject> InObject)
{
    auto DuplicatedObject = DuplicateObject(InObject);
    assert(DuplicatedObject);

    Actor = DuplicatedObject->CastToShared<MActor>();
    assert(Actor);

    W->addActor(Actor);
}

void MActorEditor::RenderUI()
{
    for (auto& [Name, Comp] : Actor->GetComponents())
    {
        // 컴포넌트 속성 편집 기능
        const FTypeDesc* Current = Comp->GetTypeDesc();
        while (Current)
        {
            if (ImGui::CollapsingHeader(Current->Name.c_str()))
            {
                DispatchStruct(Current, Comp.get());
            }

            Current = Current->Parent;
        }
    }

    //DispatchStruct(Actor->GetTypeDesc(), Actor.get());
}
