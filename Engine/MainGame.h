#pragma once

#include "Include.h"
#include "Core/Delegate.h"

class Window;
class MTimerManager;
class FrameManager;

class MMeshComponent;

class MActor;
class MCamera;

class MPhysX;

struct FPrimitiveData;

// 피킹 --------------------------------------------------------------------------
struct FHitData
{
    std::weak_ptr<class MPrimitiveComponent> HitComponent;
    Vec3 HitPos = VEC3ZERO;
    float Distance = 0.f;
    uint32 PrimitiveIndex = 0;
};

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
    virtual void PlayGame();
    FDelegate<void>& GetGamePlayedDelegate() { return OnGamePlayedDelegate; }
protected:
    bool HasBegan = false;
    FDelegate<void> OnGamePlayedDelegate;

public:	
	// 디버깅 할 때 쓰는 용도
	virtual void render();	

	// 업데이트 할 액터들을 관리
public:
	void addActor(std::shared_ptr<MActor> pActor);
protected:
	std::list<std::shared_ptr<MActor>> Actors;

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
    virtual bool IsPickable() const { return true; }
    bool Raycast(const std::vector<FPrimitiveData>& InPrimitives, FHitData& OutHitData);

public:
    bool IsMouseInViewport() const;
    void ScreenToWorld(const Vec2& InPos, float Depth, Vec3& OutPos) const;
    void WorldToScreen(const Vec3& InPos, Vec2& OutPos) const;
    void ProjectVec3(const Vec3& InBase, const Vec3& InTarget, Vec3& Out) const;
    void ProjectVec2(const Vec2& InBase, const Vec2& InTarget, Vec2& Out) const;

    const Vec2 GetMousePos() const;
};