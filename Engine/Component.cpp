#include "Include.h"
#include "Component.h"

Component::Component()
{
}


Component::~Component()
{
}

void Component::setOwningActor(std::shared_ptr<Actor> &actor)
{
	_pOwningActor = actor;
}

std::shared_ptr<Actor> Component::getOwningActor() const
{
	return _pOwningActor.lock();
}