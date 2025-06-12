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

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
	virtual void UpdateObjectConstantBuffer(const FPrimitiveData & primitiveData) override;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
};

class PointShadowDepthPass : public RenderPass
{
public:
	explicit PointShadowDepthPass();
	virtual ~PointShadowDepthPass() = default;

private:
    virtual void DrawPrimitive(const FPrimitiveData& primitiveData) override;
};

class LightPass : public RenderPass
{
public:
	explicit LightPass() = default;
	virtual ~LightPass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
	virtual void UpdateObjectConstantBuffer(const FPrimitiveData &primitiveData) override;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& primitiveData) override;
    virtual void HandleOuputMergeStage(const FPrimitiveData& primitiveData) override;
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

