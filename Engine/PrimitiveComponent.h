#pragma once
#ifndef __PRIMITIVE_COMPONENT_H__

#include "SceneComponent.h"

class RenderPass;

struct PrimitiveData;

class ENGINE_DLL PrimitiveComponent abstract : public SceneComponent, public std::enable_shared_from_this<PrimitiveComponent>
{
public:
	enum class RenderMode
	{
		Perspective, Orthogonal, End
	};	

public:
	explicit PrimitiveComponent();
	virtual ~PrimitiveComponent();

public:
	virtual void Update(const Time deltaTime) override;

public:
	virtual const bool getPrimitiveData(PrimitiveData &primitiveData);

public:
	void				setRenderMode(const RenderMode renderMode);
	const RenderMode	getRenderMdoe() const;
private:
	RenderMode _eRenderMdoe;
};

#define __PRIMITIVE_COMPONENT_H__
#endif