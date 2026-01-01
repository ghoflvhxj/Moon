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

protected:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;

protected:
    //std::weak_ptr<class MLightComponent> CachedLightComponent;
    //std::vector<Vec4> LightPosition;
    //std::vector<Mat4> LightViewProj;
};

class PointShadowDepthPass : public MRenderPass
{
public:
	explicit PointShadowDepthPass();
	virtual ~PointShadowDepthPass() = default;

private:
    virtual void RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList) override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
};

class DirectionalLightPass : public MRenderPass
{
public:
	explicit DirectionalLightPass() = default;
	virtual ~DirectionalLightPass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
	virtual void UpdateObjectConstantBuffer(const FPrimitiveData &primitiveData) override;

protected:
    virtual void HandleOutputMergeStage(const FPrimitiveData& primitiveData) override;
};

class PointLightPass : public MRenderPass
{
public:
    explicit PointLightPass() = default;
    virtual ~PointLightPass() = default;

public:
    virtual void Begin() override;
    virtual void End() override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData) override;

protected:
    virtual void HandleOutputMergeStage(const FPrimitiveData& primitiveData) override;

protected:
    uint32 PointLightIndex = 0;
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