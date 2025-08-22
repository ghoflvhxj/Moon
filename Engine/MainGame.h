#pragma once

#include "Include.h"
#include "Core/Delegate.h"
#include "Core/Object.h"

// 리플렉션을 위한 include
#include "Actor.h"

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

class ENGINE_DLL MainGame : public MObject
{
public:
	explicit MainGame();
	MainGame(const MainGame &ref) = delete;
	MainGame(MainGame &&ref) = delete;
	virtual ~MainGame();

	MainGame &operator=(const MainGame &ref) = delete;

    // 임시
public:
    virtual void OnLoaded() override
    {
        LOG(std::wstring(TEXT("Game Loaded!!!")));

        for (auto& [Name, Actor] : Actors)
        {
            Actor->PostConstruct();
        }
    }

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
    FDelegate<void>& GetGameStartedDelegate() { return OnGameStartedDelegate; }
protected:
    bool HasBegan = false;
    FDelegate<void> OnGameStartedDelegate;

public:	
	// 디버깅 할 때 쓰는 용도
	virtual void render();	

	// 업데이트 할 액터들을 관리
public:
	void addActor(std::shared_ptr<MActor> pActor);
protected:
	//std::list<std::shared_ptr<MActor>> Actors;
    std::unordered_map<std::string, std::shared_ptr<MActor>> Actors;
    uint32 ActorIndexer = 0;

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
	std::shared_ptr<MCamera> _pMainCamera = nullptr;

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

    REFLECT(
        MainGame
        , PROPERTY(Actors)
    );
};

template <class T>
std::shared_ptr<T> CreateActor(std::shared_ptr<MainGame> InGame)
{
    if (InGame == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<T> NewActor = std::make_shared<T>();
    NewActor->SetOwner(InGame);
    NewActor->PostConstruct();
    InGame->addActor(NewActor);

    return NewActor;
}