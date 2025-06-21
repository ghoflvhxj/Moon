#pragma once
#ifndef __COMBINE_PASS_H__

#include "RenderPass.h"

class CombinePass : public MRenderPass
{
public:
	explicit CombinePass();
	virtual ~CombinePass() = default;

public:
	//virtual const bool processPrimitiveData(const FPrimitiveData &primitiveData) override;

private:
    //virtual void HandleVertexShaderStage(const FPrimitiveData& PrimitiveData) override;
    //virtual void HandlePixelShaderStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOuputMergeStage(const FPrimitiveData& PrimitiveData) override;
};

class GeometryPass : public MRenderPass
{
public:
	explicit GeometryPass() = default;
	virtual ~GeometryPass() = default;

public:
	virtual bool IsValidPrimitive(const FPrimitiveData &primitiveData) const override;
};

class DirectionalShadowDepthPass : public MRenderPass
{
public:
	explicit DirectionalShadowDepthPass();
	virtual ~DirectionalShadowDepthPass() = default;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData) override;

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
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& primitiveData) override;
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
    virtual void HandleRasterizerStage(const FPrimitiveData& primitiveData) override;
    virtual void HandleOuputMergeStage(const FPrimitiveData& primitiveData) override;
};

class PointLightPass : public MRenderPass
{
public:
    explicit PointLightPass();
    virtual ~PointLightPass() = default;

public:
    virtual void End() override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData) override;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOuputMergeStage(const FPrimitiveData& primitiveData) override;

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

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
};

class CollisionPass : public MRenderPass
{
public:
    explicit CollisionPass() = default;
    virtual ~CollisionPass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
};

#define __COMBINE_PASS_H__
#endif

