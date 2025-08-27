#include "Include.h"
#include "Component.h"

Component::Component()
{
}


Component::~Component()
{
}

void Component::BeginPlay()
{
    GetBeganPlay().Broadcast();
}

void Component::setOwningActor(std::shared_ptr<MActor> &actor)
{
	_pOwningActor = actor;
}

std::shared_ptr<MActor>& Component::getOwningActor() const
{
	return _pOwningActor.lock();
}