#pragma once

#include "LightComponent.h"

class MTexture;

class ENGINE_DLL DirectionalLightComponent : public MLightComponent
{
public:
	explicit DirectionalLightComponent(void);
	virtual ~DirectionalLightComponent(void);

public:
    virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;
};
