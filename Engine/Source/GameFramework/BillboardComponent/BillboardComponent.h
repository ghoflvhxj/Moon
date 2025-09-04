#pragma once

#include "Include.h"
#include "StaticMeshComponent.h"

class StaticMesh;
class MTexture;

class ENGINE_DLL MBillboardComponent : public StaticMeshComponent
{
public:
    MBillboardComponent();

public:
    virtual Mat4& getWorldMatrix() override;
    Mat4 BillboardWorldMat = ZEROMATRIX;

public:
    virtual const bool GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList) override;

protected:
    std::shared_ptr<MTexture> Texture = nullptr;

    REFLECT(
        MBillboardComponent
    )
};