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
	std::shared_ptr<StaticMesh> getSkyMesh();

public:
	virtual const bool getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList);

private:
	void initialize();

private:
	Vec3 _baseColor;

	std::shared_ptr<StaticMesh> _pSkyMesh;
};

#define __SKY_COMPONENT_H__
#endif