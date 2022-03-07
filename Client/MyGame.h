#pragma once
#ifndef __MYGAME_H__

#include "MainGame.h"

class MeshComponent;
class TerrainComponent;
class Camera;
class Player;

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
	void controlCamera(const Time deltaTime);

private:
	std::shared_ptr<Player> _pPlayer;
private:
	std::shared_ptr<MeshComponent> _pMeshComponent;
	std::shared_ptr<TerrainComponent> _pTerrainComponent;
	//std::shared_ptr<Camera>	_pCamera;			Camera의 기본 생성자는 g_pMainGame의 Setting을 가져와 fov를 설정함, 근데 아직 g_pMainGame이 설정되기 전임!
};

#define __MYGAME_H__
#endif