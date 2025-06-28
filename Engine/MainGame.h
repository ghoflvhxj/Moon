#pragma once

#include "Include.h"

class Window;
class MTimerManager;
class FrameManager;

class MMeshComponent;

class Actor;
class MCamera;

class MPhysX;

class ENGINE_DLL MainGame : public std::enable_shared_from_this<MainGame>
{
public:
	explicit MainGame();
	MainGame(const MainGame &ref) = delete;
	MainGame(MainGame &&ref) = delete;
	virtual ~MainGame() = default;

	MainGame &operator=(const MainGame &ref) = delete;

	//-------------------------------------------------------------------------
public:
	virtual const bool initialize();

public:
	const bool Loop();
protected:
	virtual void Tick(const Time deltaTime);
private:
    void Update(const Time deltaTime);
    virtual void PostUpdate(const Time deltaTime) {}

public:	
	// 디버깅 할 때 쓰는 용도
	virtual void render();	

	// 업데이트 할 액터들을 관리
public:
	void addActor(std::shared_ptr<Actor> pActor);
private:
	std::list<std::shared_ptr<Actor>> _actorList;

	//-------------------------------------------------------------------------
public:
	const Time getDeltaTime() const;
private:
	Time _deltaTime;

public:
	const std::shared_ptr<MTimerManager> getTimerManager() const;
private:
	mutable std::shared_ptr<MTimerManager> _pTimerManager;

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
	void SetMainCamera(std::shared_ptr<MCamera> pCamera);
	std::shared_ptr<MCamera> getMainCamera() const;
public:
	const Mat4& getMainCameraViewMatrix() const;
	const Mat4& getMainCameraProjectioinMatrix() const;
	const Mat4& getMainCameraOrthographicProjectionMatrix() const;
private:
	std::shared_ptr<MCamera> _pMainCamera;

public:
    void Pick();
};
