#pragma once
#ifndef __COMBINE_PASS_H__

#include "RenderPass.h"

class CombinePass : public RenderPass
{
public:
	explicit CombinePass() = default;
	virtual ~CombinePass() = default;

public:
	virtual const bool processPrimitiveData(PrimitiveData &primitiveData) override;

private:
	void render(PrimitiveData &primitiveData);
};

class GeometryPass : public RenderPass
{
public:
	explicit GeometryPass() = default;
	virtual ~GeometryPass() = default;

public:
	virtual const bool processPrimitiveData(PrimitiveData &primitiveData) override;

private:
	void render(PrimitiveData &primitiveData);
};

class ShadowDepthPass : public RenderPass
{
public:
	explicit ShadowDepthPass() = default;
	virtual ~ShadowDepthPass() = default;

public:
	virtual const bool processPrimitiveData(PrimitiveData & primitiveData) override;

private:
	void render(PrimitiveData & primitiveData);
};

class LightPass : public RenderPass
{
public:
	explicit LightPass() = default;
	virtual ~LightPass() = default;

public:
	virtual const bool processPrimitiveData(PrimitiveData &primitiveData) override;

private:
	void render(PrimitiveData &primitiveData);
};

class SkyPass : public RenderPass
{
public:
	explicit SkyPass() = default;
	virtual ~SkyPass() = default;

public:
	virtual const bool processPrimitiveData(PrimitiveData &primitiveData) override;

private:
	void render(PrimitiveData & primitiveData);
};

#define __COMBINE_PASS_H__
#endif

