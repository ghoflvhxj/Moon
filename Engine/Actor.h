#pragma once

#include "Include.h"

class Component;
class SceneComponent;
class MPrimitiveComponent;
class MainGame;

class ENGINE_DLL Actor : public std::enable_shared_from_this<Actor>
{
public:
	explicit Actor();
	virtual ~Actor();

public:
	void update(const Time deltaTime);
protected:
	virtual void tick(const Time deltaTime);

public:
	std::shared_ptr<SceneComponent>		getComponent(const wchar_t componentName[]);
	const bool							addComponent(const wchar_t componentName[], std::shared_ptr<SceneComponent> pComponent);
private:
	std::unordered_map<std::wstring, std::shared_ptr<SceneComponent>>	_components;
};

template <class T>
std::shared_ptr<T> CreateActor(MainGame* InGame)
{
    if (InGame == nullptr)
    {
        return nullptr;
    }

    std::shared_ptr<T> NewActor = std::make_shared<T>();
    InGame->addActor(NewActor);

    return NewActor;
}