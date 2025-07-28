#pragma once

#include "MainGame.h"

class MMeshComponent;
class TerrainComponent;
class SphereComponent;
class MCamera;
class Actor;
class Player;
class StaticMeshComponent;
class MStaticMeshActor;

enum class EAxies
{
    X, Y, Z
};

class MyGame : public MainGame
{
public:
	explicit MyGame();
	virtual ~MyGame();

private:
	const bool initialize();
	void intializeImGui();

protected:
	virtual void Tick(const Time deltaTime) override;
    virtual void PostUpdate(const Time deltaTime) override;

	virtual void render() override;

private:
    std::shared_ptr<MStaticMeshActor> LanternActor;
    std::shared_ptr<MStaticMeshActor> ClothActor;

private:
	std::shared_ptr<Player> _pPlayer;

private:
	//std::shared_ptr<TerrainComponent> _pTerrainComponent;
	
	float Force = 0.f;

public:
    bool IsPickable() const;
private:
    FHitData HitData = {};
    Vec3 GizmoOffset = VEC3ZERO;
    EAxies GizmoAxis;
    bool bUpdateGizmoOffset = false;
	bool bControlGizmo = false;


    std::weak_ptr<class Component> ClickedComp;
};

void DispatchContainer(const FTypeDesc* InElementTypeDesc, FContainerPropertyDesc* InContainerDesc, void* InObject);
void DispatchStruct(const FTypeDesc* InStructDesc, void* InObject);