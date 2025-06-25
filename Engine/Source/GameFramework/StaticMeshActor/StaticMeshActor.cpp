#include "StaticMeshActor.h"
#include "StaticMeshComponent.h"

MStaticMeshActor::MStaticMeshActor()
    : Actor()
{
    StaticMeshComp = std::make_shared<StaticMeshComponent>();
    addComponent(ROOT_COMPONENT, StaticMeshComp);
}

void MStaticMeshActor::SetStaticMesh(const std::wstring& Path)
{
    StaticMeshComp->SetMesh(Path);
}

