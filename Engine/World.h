#pragma once

#include "Include.h"
#include "Core/Delegate.h"
#include "Core/Object.h"

#include "Module/Physics/PhysicsEnum.h"

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
    float Distance = FLT_MAX;
    uint32 PrimitiveIndex = 0;

    bool IsValid() const
    {
        return Distance != FLT_MAX;
    }
};

enum class EWorldType
{
    None,
    Play,
    Editor,
    PIayInEditor,
    WorkInEditor
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

public:
    virtual void OnLoaded() override;
    virtual std::shared_ptr<MObject> Duplicate() override;    // Map 복제를 아직 지원안해서 수동으로 작성해줘야 함
    void DuplicateActors(const std::shared_ptr<MWorld>& InSrcWorld);

public:
	virtual const bool Initialize();
    virtual void render();

public:
    // 액터들을 업데이트 함
	bool Update();

public:
    virtual void PlayGame();
    FDelegate<void>& GetGameStartedDelegate() { return OnGameStartedDelegate; }
    bool IsHasBegan() const { return bHasBegan; }
protected:
    bool bHasBegan = false;
    FDelegate<void> OnGameStartedDelegate;

public:
    FDelegate<void>& GetOnRederedDelegate() { return OnRendered; }
protected:
    FDelegate<void> OnRendered;

public:
    void SetWorldType(EWorldType InWorldType) { WorldType = InWorldType; }
    bool IsWorldType(EWorldType InWorldType) const { return WorldType == InWorldType; }
protected:
    EWorldType WorldType = EWorldType::None;

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
    bool Raycast(FHitData& OutHitData, ECollisionType InCollisionType);
    bool Raycast(const std::vector<FPrimitiveData>& InPrimitives, FHitData& OutHitData, uint8 InPrimitiveType);

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

    std::shared_ptr<T> NewActor(static_cast<T*>(CreateObject(T::GetTypeDescStatic())));
    NewActor->SetOwner(InWorld);
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

    std::shared_ptr<MActor> NewActor(static_cast<MActor*>(CreateObject(InTypeDesc)));
    NewActor->SetOwner(InWorld);
    NewActor->PostConstruct();
    InWorld->addActor(NewActor);

    return NewActor;
}