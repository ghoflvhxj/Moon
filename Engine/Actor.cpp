#include "stdafx.h"
#include "Actor.h"

#include "MapUtility.h"

#include "Component.h"
#include "SceneComponent.h"

Actor::Actor()
	: _components()
{
}

Actor::~Actor()
{
}

void Actor::update(const Time deltaTime)
{
	tick(deltaTime);

	for (auto iter = _components.begin(); iter != _components.end(); ++iter)
	{
		if (false == iter->second->isUpdateable())
			continue;

		iter->second->Update(deltaTime);
	}
}

void Actor::tick(const Time deltaTime)
{
}

std::shared_ptr<SceneComponent> Actor::getComponent(const wchar_t componentName[])
{
	std::shared_ptr<SceneComponent> pComponent = nullptr;
	MapUtility::FindGet(_components, componentName, pComponent);

	return pComponent;
}

const bool Actor::addComponent(const wchar_t componentName[], std::shared_ptr<SceneComponent> pComponent)
{
	return MapUtility::FindInsert(_components, componentName, pComponent);
}


