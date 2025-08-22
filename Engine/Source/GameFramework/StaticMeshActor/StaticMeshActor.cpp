#include "StaticMeshActor.h"
#include "StaticMeshComponent.h"

MStaticMeshActor::MStaticMeshActor()
    : MActor()
{
    StaticMeshComp = std::make_shared<StaticMeshComponent>();
    AddComponent(ROOT_COMPONENT, StaticMeshComp);
}

void MStaticMeshActor::SetStaticMesh(const std::wstring& Path)
{
    StaticMeshComp->SetMesh(Path);
}

