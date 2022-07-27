#pragma once
#ifndef __SKY_COMPONENT_H__

#include "PrimitiveComponent.h"

struct PrimitiveData;
class StaticMesh;
class Material;
class ENGINE_DLL SkyComponent : public PrimitiveComponent
{
public:
	explicit SkyComponent();
	virtual ~SkyComponent();

public:
	virtual const bool getPrimitiveData(PrimitiveData &primitiveData);

private:
	void initialize();

private:
	Vec3 _baseColor;

	std::shared_ptr<StaticMesh> _pSkyMesh;
};

#define __SKY_COMPONENT_H__
#endif