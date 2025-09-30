#pragma once

#include "Actor.h"

class StaticMeshComponent;

class ENGINE_DLL MStaticMeshActor : public MActor
{
public:
    MStaticMeshActor();

public:
    virtual void tick(const Time deltaTime) override;

public:
    void SetStaticMesh(const std::wstring& Path);
    std::shared_ptr<StaticMeshComponent>& GetStaticMeshCompoent() { return StaticMeshComp; }
protected:
    std::shared_ptr<StaticMeshComponent> StaticMeshComp = nullptr;

    void QuaternionToEuler_XYZ(Vec4 q, float& outPitch, float& outYaw, float& outRoll);

    REFLECT(MStaticMeshActor)
};