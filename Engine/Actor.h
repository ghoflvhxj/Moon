#pragma once

#include "Include.h"
#include "Core/Object.h"
#include "Core/Delegate.h"

// 리플렉션을 위한 include
#include "SceneComponent.h"

class Component;
class SceneComponent;
class MPrimitiveComponent;
class MWorld;

class ENGINE_DLL MActor : public MObject
{
public:
	explicit MActor();
	virtual ~MActor();

    // 생성자 호출 후 처리할 작업
    void PostConstruct();

public:
    virtual void BeginPlay();
    FDelegate<void, std::shared_ptr<MActor>>& GetBeganPlayDelegate() { return OnBeganPlayDelegate; }
    bool HasBegan() const { return bHasBegan; }
protected:
    FDelegate<void, std::shared_ptr<MActor>> OnBeganPlayDelegate;
    bool bHasBegan = false;

public:
	void update(const Time deltaTime);
protected:
	virtual void tick(const Time deltaTime);

public:
    const Vec3 GetWorldTranslation();
    void SetWorldTranslation(const Vec3& InTrans);

public:
    std::unordered_map<std::wstring, std::shared_ptr<SceneComponent>>& GetComponents() { return _components; }
	std::shared_ptr<SceneComponent>&	getComponent(const wchar_t componentName[]);
	const bool							AddComponent(const wchar_t componentName[], std::shared_ptr<SceneComponent> InComponent);
protected:
	std::unordered_map<std::wstring, std::shared_ptr<SceneComponent>>	_components;

    REFLECT(
        MActor
        , PROPERTY(_components)
    );
};