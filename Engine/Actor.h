#pragma once

#include "Include.h"
#include "Core/Object.h"
#include "Core/Delegate.h"

// 리플렉션을 위한 include
#include "SceneComponent.h"

class MComponent;
class MSceneComponent;
class MPrimitiveComponent;
class MWorld;

struct ENGINE_DLL FAttachData
{
    std::wstring ParentName;
    std::wstring ChildName;

    REFLECT_TOP(
        FAttachData
        , PROPERTY(ParentName)
        , PROPERTY(ChildName)
    )
};

class ENGINE_DLL MActor : public MObject
{
public:
	explicit MActor();
	virtual ~MActor();

    virtual void PostConstruct() override;
    virtual void OnLoaded() override; 
    virtual void OnDuplicated(MObject* SrcObject) override;

    void RegistComponents();

public:
    virtual void BeginPlay();
    FDelegate<void, std::shared_ptr<MActor>>& GetBeganPlayDelegate() { return OnBeganPlayDelegate; }
    bool HasBegan() const { return bHasBegan; }
protected:
    FDelegate<void, std::shared_ptr<MActor>> OnBeganPlayDelegate;
    bool bHasBegan = false;

public:
    virtual void Destroy();

public:
    MWorld* GetWorld();

public:
	void update(const Time deltaTime);
protected:
	virtual void tick(const Time deltaTime);

public:
    const Vec3 GetWorldTranslation();
    void SetWorldTranslation(const Vec3& InTrans);

public:
    std::unordered_map<std::wstring, std::shared_ptr<MSceneComponent>>& GetComponents() { return SceneComponents; }
    std::shared_ptr<MSceneComponent>& getComponent(const wchar_t componentName[]);
    std::shared_ptr<MSceneComponent>& getComponent(const std::wstring& InName);
    bool AddComponent(const std::wstring& InName, std::shared_ptr<MSceneComponent> InComponent);
	bool AddComponent(const wchar_t componentName[], std::shared_ptr<MSceneComponent> InComponent);
protected:
	std::unordered_map<std::wstring, std::shared_ptr<MSceneComponent>>	SceneComponents;
    std::vector<FAttachData> AttachDatas;

public:
    bool IsUpdatable() const;
protected:
    bool bUpdatable = true;

    REFLECT(
        MActor
        , PROPERTY(AttachDatas)
        , PROPERTY(bUpdatable)
    );
};