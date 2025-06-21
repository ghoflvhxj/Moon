#pragma once
#ifndef __SKY_COMPONENT_H__

#include "PrimitiveComponent.h"

struct FPrimitiveData;
class StaticMesh;
class MMaterial;
class ENGINE_DLL SkyComponent : public MPrimitiveComponent
{
public:
	explicit SkyComponent();
	virtual ~SkyComponent();

public:
	std::shared_ptr<StaticMesh> getSkyMesh();

public:
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList);

private:
	void initialize();

private:
	Vec3 _baseColor;

	std::shared_ptr<StaticMesh> _pSkyMesh;
};

#define __SKY_COMPONENT_H__
#endif