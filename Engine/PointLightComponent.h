#pragma once
#ifndef __POINTLIGHT_COMPONENT_H__

#include "LightComponent.h"

struct FPrimitiveData;

class ENGINE_DLL PointLightComponent : public LightComponent
{
public:
	explicit PointLightComponent(void);
	virtual ~PointLightComponent(void);


public:
	virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;

public:
	void		addRange(const float addRange);
	void		setRange(const float range);
	const float	getRange() const;
private:
	float	_range;
};

#define __POINTLIGHT_COMPONENT_H__
#endif