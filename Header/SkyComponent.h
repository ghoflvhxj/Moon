#pragma once
#ifndef __SKY_COMPONENT_H__

#include "SceneComponent.h"

class StaticMeshComponent;

class ENGINE_DLL SkyComponent : public SceneComponent, public std::enable_shared_from_this<SkyComponent>
{
public:
	explicit SkyComponent();
	~SkyComponent();
private:
	void initialize();

public:
	void Update(const Time deltaTime) override;
public:
	void render();

public:
	void Test(ID3D11ShaderResourceView * p);

private:
	Vec3 _baseColor;

	std::shared_ptr<StaticMeshComponent> _pStaticMeshComponent;
};

#define __SKY_COMPONENT_H__
#endif