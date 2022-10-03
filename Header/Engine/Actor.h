#pragma once
#ifndef __ACTOR_H__

class Component;
class SceneComponent;
class PrimitiveComponent;
class MainGame;

class ENGINE_DLL Actor : public std::enable_shared_from_this<Actor>
{
public:
	explicit Actor();
	~Actor();

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

#define __ACTOR_H__
#endif