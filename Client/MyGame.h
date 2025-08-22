#pragma once

#include "MainGame.h"

class MMeshComponent;
class TerrainComponent;
class SphereComponent;
class MCamera;
class MActor;
class Player;
class StaticMeshComponent;
class MStaticMeshActor;

enum class EAxies
{
    X, Y, Z
};

enum class EGizmoMode
{
    Trans,
    Rot,
    Scale,
    Count
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
    bool bSetGizmoOffset = false;
	bool bControlGizmo = false;
    EGizmoMode GizmoMode = EGizmoMode::Trans;
    Vec3 Prev = {};

    std::weak_ptr<class Component> ClickedComp;

public:
    const FTypeDesc* EditAssetDesc = nullptr;
    class MAsset* EditAsset = nullptr;
};

void DispatchContainer(const FTypeDesc* InElementTypeDesc, FVectorPropertyDesc* InContainerDesc, void* InObject);
void DispatchArray(const FTypeDesc* InElementTypeDesc, FPropertyDesc* InPropertyDesc, void* InObject);
void DispatchStruct(const FTypeDesc* InStructDesc, void* InObject);


void HandleProperty(EType InType, const char* DisplayName, void* InData);