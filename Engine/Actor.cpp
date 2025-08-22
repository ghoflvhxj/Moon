#include "Actor.h"

#include "MoonEngine.h"
#include "MapUtility.h"

#include "Component.h"
#include "SceneComponent.h"

MActor::MActor()
	: _components()
{
}

MActor::~MActor()
{
    // DoNothing
}

void MActor::PostConstruct()
{
    for (auto& [Name, Comp] : _components)
    {
        RegisterComponent(Comp);
        Comp->SetOwner(GetShared());
        Comp->setOwningActor(GetShared());
    }
}

void MActor::BeginPlay()
{
    OnBeganPlayDelegate.Broadcast(GetShared());

    for (auto& [Name, Comp] : _components)
    {
        Comp->BeginPlay();
    }
}

void MActor::update(const Time deltaTime)
{
	tick(deltaTime);

	for (auto iter = _components.begin(); iter != _components.end(); ++iter)
	{
		if (false == iter->second->isUpdateable())
			continue;

		iter->second->Update(deltaTime);
	}

	for (auto iter = _components.begin(); iter != _components.end(); ++iter)
	{
		iter->second->OnUpdated();
	}
}

void MActor::tick(const Time deltaTime)
{
}

const Vec3 MActor::GetWorldTranslation()
{
    if (auto& RootComp = getComponent(ROOT_COMPONENT))
    {
        return RootComp->getWorldTranslation();
    }

    return VEC3ZERO;
}

std::shared_ptr<SceneComponent> MActor::getComponent(const wchar_t componentName[])
{
	std::shared_ptr<SceneComponent> pComponent = nullptr;
	MapUtility::FindGet(_components, componentName, pComponent);

	return pComponent;
}

const bool MActor::AddComponent(const wchar_t componentName[], std::shared_ptr<SceneComponent> InComponent)
{
	return MapUtility::FindInsert(_components, componentName, InComponent);
}