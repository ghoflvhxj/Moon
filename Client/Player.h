#pragma once
#ifndef __PLAYER_H__

#include "Actor.h"

class MeshComponent;
class StaticMeshComponent;
class DynamicMeshComponent;
class TerrainComponent;
class TextureComponent;
class PointLightComponent;
class DirectionalLightComponent;
class SkyComponent;
class CollisionShapeComponent;

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
	void rideTerrain(std::shared_ptr<TerrainComponent> pTerrainComponent);

private:
	std::shared_ptr<MeshComponent>			_pMeshComponent;
	std::shared_ptr<StaticMeshComponent>	_pStaticMeshComponent;
	std::shared_ptr<StaticMeshComponent>	_pStaticMeshComponent2;
	std::shared_ptr<DynamicMeshComponent>	_pDynamicMeshComponent;
	std::shared_ptr<SkyComponent>			_pSkyComponent;
	std::shared_ptr<TextureComponent>		_pTextureComponent;
	std::shared_ptr<PointLightComponent>	_pLightComponent;
	std::shared_ptr<DirectionalLightComponent>	_pLightComponent2;
	std::shared_ptr<CollisionShapeComponent>_pCollisionShapeComponent;
	std::shared_ptr<CollisionShapeComponent>_pBoneShapeComponent;

private:
	std::vector<std::shared_ptr<PointLightComponent>>	_pLightComponentList;
};

#define __PLAYER_H__
#endif