#pragma once

#include "Include.h"
#include "Render.h"
#include "StaticMeshComponent.h"

class StaticMesh;
class MTexture;

class ENGINE_DLL MBillboardComponent : public StaticMeshComponent
{
public:
    MBillboardComponent();

public:
    virtual void Update(const Time deltaTime) override;
    virtual void OnRegisted() override;

public:
    virtual const bool GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList) override;

public:
    //std::shared_ptr<MTexture> Texture = nullptr;
    std::shared_ptr<class MMaterial> Material = nullptr;

public:
    void SetPrimitiveType(EPrimitiveType InType);
protected:
    EPrimitiveType PrimitiveType = EPrimitiveType::Mesh;

public:
    bool bAlwaysUpdate = false;

    REFLECT(MBillboardComponent
        , PROPERTY(PrimitiveType)
    )
};