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
    virtual void Update(float DeltaTime) override;
    virtual void Render() override;
    std::shared_ptr<class DynamicMeshComponent> GetDynamicMeshComponent();
    std::shared_ptr<class DynamicMesh> GetDynamicMesh();

protected:
    FBodyCapsuleData BodyCapsuleData;
};