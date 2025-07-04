#pragma once

#include "Actor.h"

class MMeshComponent;
class StaticMeshComponent;
class DynamicMeshComponent;
class TerrainComponent;
class MTexture;
class PointLightComponent;
class DirectionalLightComponent;
class SkyComponent;

class Player : public Actor
{
public:
	explicit Player();
	virtual ~Player();

private:
	void initialize();
	void initializeImGui();
protected:
	virtual void tick(const Time deltaTime) override;

public:
	void JsonSaveTest(bool bPretty = false);
    void JsonLoadTest();
	//void rideTerrain(std::shared_ptr<TerrainComponent> pTerrainComponent);

private:
	std::shared_ptr<StaticMeshComponent>	_pMeshComponent;
	std::shared_ptr<StaticMeshComponent>	_pStaticMeshComponent;
    std::shared_ptr<StaticMeshComponent>	_pStaticMeshComponent2;
    std::shared_ptr<StaticMeshComponent>	LoadedMeshComponent;
	std::shared_ptr<DynamicMeshComponent>	CharacterMeshComponent;
	std::shared_ptr<SkyComponent>			_pSkyComponent;
	std::shared_ptr<PointLightComponent>	_pLightComponent;
	std::shared_ptr<DirectionalLightComponent>	_pLightComponent2;

private:
	std::vector<std::shared_ptr<PointLightComponent>>	_pLightComponentList;


private:
    float CameraSpeedScale = 1.f;

