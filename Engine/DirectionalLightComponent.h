#pragma once
#ifndef __DIRECTIONAL_LIGHT_COMPONENT_H__

#include "LightComponent.h"

#include "Vertex.h"

class VertexBuffer;
class IndexBuffer;
class MConstantBuffer;

class MTexture;

class ENGINE_DLL DirectionalLightComponent : public LightComponent
{
public:
	explicit DirectionalLightComponent(void);
	virtual ~DirectionalLightComponent(void);

public:
	virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;

private:
	Vec3 _forward;
};

#define __DIRECTIONAL_LIGHT_COMPONENT_H__
#endif