#pragma once

#include "RenderPass.h"

class GeometryPass : public MRenderPass
{
public:
    explicit GeometryPass();
	virtual ~GeometryPass() = default;

protected:

	virtual bool IsValidPrimitive(const FPrimitiveData &primitiveData) const override;
};

class DirectionalShadowDepthPass : public MRenderPass
{
public:
	explicit DirectionalShadowDepthPass();
	virtual ~DirectionalShadowDepthPass() = default;

public:
    virtual void RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList) override;

protected:
    virtual void DrawPrimitive(const FPrimitiveData& PrimitiveData) override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData) override;

public:
    const std::vector<Mat4>& GetViewProjs() const;
protected:
    std::vector<Mat4> ViewProj;
    std::vector<Mat4> Transforms;

protected:
    std::shared_ptr<class MVertexBuffer> InstanceBuffer;
};

class PointShadowDepthPass : public MRenderPass
{
public:
	explicit PointShadowDepthPass();
	virtual ~PointShadowDepthPass() = default;

public:
    virtual void RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList) override;

protected:
    virtual void DrawPrimitive(const FPrimitiveData& PrimitiveData) override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData) override;

protected:
    uint32 PointLightIndex = 0;
    Vec3 LightPos = VEC3ZERO;
protected:
    std::vector<Mat4> ViewProj;
    std::vector<Mat4> Transforms;

protected:
    std::shared_ptr<class MVertexBuffer> InstanceBuffer;
};

class SkyPass : public MRenderPass
{
public:
	explicit SkyPass() = default;
	virtual ~SkyPass() = default;

public:
	virtual bool IsValidPrimitive(const FPrimitiveData &primitiveData) const override;
};

class MLinePass : public MRenderPass
{
public:
    explicit MLinePass();
    virtual ~MLinePass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
};

class MDepthPre : public MRenderPass
{
public:
    explicit MDepthPre();

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;    
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
};