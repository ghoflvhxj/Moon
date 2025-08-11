#pragma once

#include "Include.h"
#include "Core/Object.h"
#include "Core/Delegate.h"

class Component;
class SceneComponent;
class MPrimitiveComponent;
class MainGame;

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
protected:
    FDelegate<void, std::shared_ptr<MActor>> OnBeganPlayDelegate;

public:
	void update(const Time deltaTime);
protected:
	virtual void tick(const Time deltaTime);

public:
    const Vec3 GetWorldTranslation();

public:
    std::unordered_map<std::wstring, std::shared_ptr<SceneComponent>>& GetComponents() { return _components; }
	std::shared_ptr<SceneComponent>		getComponent(const wchar_t componentName[]);
	const bool							addComponent(const wchar_t componentName[], std::shared_ptr<SceneComponent> pComponent);
private:
	std::unordered_map<std::wstring, std::shared_ptr<SceneComponent>>	_components;

    REFLECT(MActor)
};

template <class T>
std::shared_ptr<T> CreateActor(MainGame* InGame)
{
    if (InGame == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<T> NewActor = std::make_shared<T>();
    NewActor->PostConstruct();
    InGame->addActor(NewActor);

    return NewActor;
}