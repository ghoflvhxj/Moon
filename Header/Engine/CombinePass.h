#pragma once
#ifndef __COMBINE_PASS_H__

#include "RenderPass.h"

class CombinePass : public RenderPass
{
	using Super = RenderPass;

public:
	explicit CombinePass();
	virtual ~CombinePass();

public:
	virtual void doPass(RenderQueue &renderQueue) override;

private:
	void render(PrimitiveData &primitiveData);
};

class GeometryPass : public RenderPass
{
	using Super = RenderPass;

public:
	explicit GeometryPass();
	virtual ~GeometryPass();

public:
	virtual void doPass(RenderQueue &renderQueue) override;

private:
	void render(PrimitiveData &primitiveData);
};

class LightPass : public RenderPass
{
	using Super = RenderPass;

public:
	explicit LightPass() = default;
	virtual ~LightPass() = default;

public:
	virtual void doPass(RenderQueue &renderQueue) override;

private:
	void render(PrimitiveData &primitiveData);
};

#define __COMBINE_PASS_H__
#endif

