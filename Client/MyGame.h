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
	//std::shared_ptr<Camera>	_pCamera;			Camera�� �⺻ �����ڴ� g_pMainGame�� Setting�� ������ fov�� ������, �ٵ� ���� g_pMainGame�� �����Ǳ� ����!
};

#define __MYGAME_H__
#endif