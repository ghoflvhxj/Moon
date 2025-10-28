#pragma once

#include "Include.h"
#include "Core/Delegate.h"
#include "Core/Object.h"

// 리플렉션을 위한 include
#include "Actor.h"

class MWindow;
class MTimerManager;
class MFrameManager;

class MMeshComponent;

class MActor;
class MCamera;

struct FPrimitiveData;

// 피킹 --------------------------------------------------------------------------
struct FHitData
{
    std::weak_ptr<class MPrimitiveComponent> HitComponent;
    Vec3 HitPos = VEC3ZERO;
    float Distance = 0.f;
    uint32 PrimitiveIndex = 0;
};

class ENGINE_DLL MWorld : public MObject
{
public:
	explicit MWorld();
	MWorld(const MWorld &ref) = delete;
	MWorld(MWorld &&ref) = delete;
	virtual ~MWorld();

	MWorld &operator=(const MWorld &ref) = delete;

public:
    uint32 GetID() const { return ID; }
protected:
    uint32 ID = 0;

    // 임시
public:
    virtual void OnLoaded() override
    {
        LOG(std::wstring(TEXT("World Loaded!!!")));

        for (auto& [Name, Actor] : Actors)
        {
            Actor->SetOwner(GetShared());
            Actor->PostConstruct();
            Actor->update(0.f);
        }
    }

public:
	virtual const bool Initialize();
    virtual void render();

public:
    // 액터들을 업데이트 함
	bool Update();

public:
    virtual void PlayGame();
    FDelegate<void>& GetGameStartedDelegate() { return OnGameStartedDelegate; }
protected:
    bool HasBegan = false;
    FDelegate<void> OnGameStartedDelegate;

public:
    FDelegate<void>& GetOnRederedDelegate() { return OnRendered; }
protected:
    FDelegate<void> OnRendered;


	// 업데이트 할 액터들을 관리
public:
	void addActor(std::shared_ptr<MActor> pActor);
    std::unordered_map<std::string, std::shared_ptr<MActor>>& GetActors() { return Actors; }
protected:
    std::unordered_map<std::string, std::shared_ptr<MActor>> Actors;

    // 액터 이름용
    std::unordered_map<const FTypeDesc*, uint32> Indexer;
    //uint32 ActorIndexer = 0;

	//-------------------------------------------------------------------------
public:
	const Time getDeltaTime() const;
public:
	Time _deltaTime;

public:
	std::shared_ptr<MTimerManager>& getTimerManager() const;
private:
	mutable std::shared_ptr<MTimerManager> _pTimerManager;

public:
    std::shared_ptr<MFrameManager>& getFrameManager() const;
	const Frame getFrame() const;
private:
    mutable std::shared_ptr<MFrameManager> _pFrameManager;

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
    bool IsForegorund() const;
    bool IsMouseInViewport() const;
    void ScreenToWorld(const Vec2& InPos, float Depth, Vec3& OutPos) const;
    void WorldToScreen(const Vec3& InPos, Vec2& OutPos) const;
    void ProjectVec3(const Vec3& InBase, const Vec3& InTarget, Vec3& Out) const;
    void ProjectVec2(const Vec2& InBase, const Vec2& InTarget, Vec2& Out) const;

    REFLECT(
        MWorld
        , PROPERTY(Actors)
    );
};

template <class T>
std::shared_ptr<T> CreateActor(std::shared_ptr<MWorld> InWorld)
{
    if (InWorld == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<T> NewActor = std::make_shared<T>();
    NewActor->SetOwner(InWorld);
    NewActor->PostConstruct();
    InWorld->addActor(NewActor);

    return NewActor;
}

inline std::shared_ptr<MActor> CreateActor(std::shared_ptr<MWorld> InWorld, const FTypeDesc* InTypeDesc)
{
    if (InWorld == nullptr)
    {
        return nullptr;
    }

    if (InTypeDesc == nullptr)
    {
        return nullptr;
    }

    if (InTypeDesc->IsA<MActor>() == false)
    {
        return nullptr;
    }

    std::shared_ptr<MActor> NewActor(static_cast<MActor*>(Create(InTypeDesc)));
    NewActor->SetOwner(InWorld);
    NewActor->PostConstruct();
    InWorld->addActor(NewActor);

    return NewActor;
}