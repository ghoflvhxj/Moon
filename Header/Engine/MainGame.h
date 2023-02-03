#pragma once
#ifndef __MAINGAME_H__
#include "Vertex.h"

class Window;
class TimerManager;
class FrameManager;

class MeshComponent;

class Actor;
class Camera;

class PhysXX;

class ENGINE_DLL MainGame : public std::enable_shared_from_this<MainGame>
{
public:
	explicit MainGame();
	MainGame(const MainGame &ref) = delete;
	MainGame(MainGame &&ref) = delete;
	~MainGame() = default;

	MainGame &operator=(const MainGame &ref) = delete;

	//-------------------------------------------------------------------------
public:
	const bool initialize();

public:
	const bool Loop();
protected:
	virtual void Tick(const Time deltaTime);
private:
	void Update(const Time deltaTime);

public:	
	// ����� �� �� ���� �뵵
	virtual void render();	

	// ������Ʈ �� ���͵��� ����
public:
	void addActor(std::shared_ptr<Actor> pActor);
private:
	std::list<std::shared_ptr<Actor>> _actorList;

	//-------------------------------------------------------------------------
public:
	const Time getDeltaTime() const;
private:
	Time _deltaTime;	// ������ ��ŸŸ�Ӱ� Ÿ�̸��� ��ŸŸ���� ������ �ٸ�...! �����Ӱ��� ��ŸŸ�� vs while �������� ��ŸŸ��

public:
	const std::shared_ptr<TimerManager> getTimerManager() const;
private:
	mutable std::shared_ptr<TimerManager> _pTimerManager;

public:
	const std::shared_ptr<FrameManager> getFrameManager() const;
	const Frame getFrame() const;
private:
	std::shared_ptr<FrameManager> _pFrameManager;

	//-------------------------------------------------------------------
//public:
//	const std::shared_ptr<MainGameSetting> getSetting();
//private:
//	std::shared_ptr<MainGameSetting> _pMainGameSetting;


public:
	void SetMainCamera(std::shared_ptr<Camera> pCamera);
	std::shared_ptr<Camera> getMainCamera() const;
public:
	const Mat4& getMainCameraViewMatrix() const;
	const Mat4& getMainCameraProjectioinMatrix() const;
	const Mat4& getMainCameraOrthographicProjectionMatrix() const;
private:
	std::shared_ptr<Camera> _pMainCamera;
};

#define __MAINGAME_H__
#endif