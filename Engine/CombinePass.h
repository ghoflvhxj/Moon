#pragma once
#ifndef __COMBINE_PASS_H__

#include "RenderPass.h"

class CombinePass : public RenderPass
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

class GeometryPass : public RenderPass
{
public:
	explicit GeometryPass() = default;
	virtual ~GeometryPass() = default;

public:
	virtual bool IsValidPrimitive(const FPrimitiveData &primitiveData) const override;
};

class DirectionalShadowDepthPass : public RenderPass
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

class PointShadowDepthPass : public RenderPass
{
public:
	explicit PointShadowDepthPass();
	virtual ~PointShadowDepthPass() = default;

private:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& primitiveData) override;
};

class DirectionalLightPass : public RenderPass
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

class PointLightPass : public RenderPass
{
public:
    explicit PointLightPass();
    virtual ~PointLightPass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData) override;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOuputMergeStage(const FPrimitiveData& primitiveData) override;
};

class CollisionPass : public RenderPass
{
public:
    explicit CollisionPass() = default;
    virtual ~CollisionPass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
};

class SkyPass : public RenderPass
{
public:
	explicit SkyPass() = default;
	virtual ~SkyPass() = default;

public:
	virtual bool IsValidPrimitive(const FPrimitiveData &primitiveData) const override;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
};

#define __COMBINE_PASS_H__
#endif

