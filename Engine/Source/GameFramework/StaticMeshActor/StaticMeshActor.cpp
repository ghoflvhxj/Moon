#include "StaticMeshActor.h"
#include "StaticMeshComponent.h"

#include "MoonEngine.h"
#include "World.h"

#include "DynamicMeshComponent.h"

using namespace DirectX;

MStaticMeshActor::MStaticMeshActor()
    : MActor()
{
    StaticMeshComp = std::make_shared<StaticMeshComponent>();
    AddComponent(ROOT_COMPONENT, StaticMeshComp);
}

void MStaticMeshActor::tick(const Time deltaTime)
{
    Super::tick(deltaTime);
}

void MStaticMeshActor::SetStaticMesh(const std::wstring& Path)
{
    StaticMeshComp->SetMesh(Path);
}

