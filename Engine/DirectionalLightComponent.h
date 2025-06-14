#pragma once
#ifndef __DIRECTIONAL_LIGHT_COMPONENT_H__

#include "LightComponent.h"

#include "Vertex.h"

class MVertexBuffer;
class MIndexBuffer;
class MConstantBuffer;

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

#define __DIRECTIONAL_LIGHT_COMPONENT_H__
#endif