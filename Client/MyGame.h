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
public:
	virtual void render() override;

private:
    std::shared_ptr<MStaticMeshActor> LanternActor;

private:
	std::shared_ptr<Player> _pPlayer;

private:
	std::shared_ptr<TerrainComponent> _pTerrainComponent;
	//std::shared_ptr<Camera>	_pCamera;			Camera의 기본 생성자는 g_pMainGame의 Setting을 가져와 fov를 설정함, 근데 아직 g_pMainGame이 설정되기 전임!

	float time = 0.f;
	
	bool bButtonPressed = false;
	bool bStaticCollision = true;
	float Force = 0.f;
	std::shared_ptr<StaticMeshComponent> Lantern;
};