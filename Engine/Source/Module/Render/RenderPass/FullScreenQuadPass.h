#pragma once 

#include "Include.h"
#include "Render.h"
#include "RenderPass.h"

class StaticMeshComponent;

class ENGINE_DLL MFullScreenQuadPass : public MRenderPass
{
public:
    MFullScreenQuadPass();
    virtual ~MFullScreenQuadPass() = default;

public:
    virtual void RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList) override;\

protected:
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData);

protected:
    std::shared_ptr<StaticMeshComponent> ViewMeshComponent;
    std::vector<FPrimitiveData> ViewPrimitiveData;
};

class ENGINE_DLL MCombinePass : public MFullScreenQuadPass
{
public:
    MCombinePass() = default;
    virtual ~MCombinePass() = default;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData) override;
};

class ENGINE_DLL MStencilPass : public MFullScreenQuadPass
{
public:
    MStencilPass() = default;
    virtual ~MStencilPass() = default;

protected:
    //virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const;

protected:
    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData) override;
};