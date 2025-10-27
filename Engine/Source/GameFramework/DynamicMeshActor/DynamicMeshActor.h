#pragma once

#include "Actor.h"

class DynamicMeshComponent;
class DynamicMesh;

class ENGINE_DLL MDynamicMeshActor : public MActor
{
public:
    MDynamicMeshActor();

public:
    void SetDynamicMesh(const std::wstring& InPath);

protected:
    std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = nullptr;

public:
    REFLECT(MDynamicMeshActor)
};