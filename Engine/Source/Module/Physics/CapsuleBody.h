#pragma once

#include "Module/Physics/Jolt.h"

class ENGINE_DLL MCapsuleBody : public MBodyObject
{
public:
    MCapsuleBody(const FBodyConstructData& InData, const FBodyCapsuleData& InCapsuleData)
        : MBodyObject(InData)
        , BodyCapsuleData(InCapsuleData)
    {

    }

public:
    void Update(float DeltaTime);
    void Render();
    std::shared_ptr<class DynamicMeshComponent> GetDynamicMeshComponent();
    std::shared_ptr<class DynamicMesh> GetDynamicMesh();

protected:
    FBodyCapsuleData BodyCapsuleData;
};