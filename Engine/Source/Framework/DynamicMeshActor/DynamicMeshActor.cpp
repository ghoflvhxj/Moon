#include "DynamicMeshActor.h"

#include "DynamicMeshComponent.h"

MDynamicMeshActor::MDynamicMeshActor()
{
    DynamicMeshComp = std::make_shared<DynamicMeshComponent>();
    AddComponent(ROOT_COMPONENT, DynamicMeshComp);
}

void MDynamicMeshActor::SetDynamicMesh(const std::wstring& InPath)
{
    DynamicMeshComp->SetMesh(InPath);
}
