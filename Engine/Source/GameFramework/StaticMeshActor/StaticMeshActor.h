#pragma once

#include "Actor.h"

class StaticMeshComponent;

class ENGINE_DLL MStaticMeshActor : public Actor
{
public:
    MStaticMeshActor();

public:
    void SetStaticMesh(const std::wstring& Path);
    std::shared_ptr<StaticMeshComponent> GetStaticMeshCompoent() { return StaticMeshComp; }
protected:
    std::shared_ptr<StaticMeshComponent> StaticMeshComp = nullptr;
};