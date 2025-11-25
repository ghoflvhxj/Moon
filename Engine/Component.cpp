#include "Include.h"
#include "Component.h"

MComponent::MComponent()
{
}


MComponent::~MComponent()
{
}

void MComponent::BeginPlay()
{
    GetBeganPlay().Broadcast();
}

void MComponent::setOwningActor(std::shared_ptr<MActor> &actor)
{
	_pOwningActor = actor;
}

std::shared_ptr<MActor> MComponent::getOwningActor() const
{
	return _pOwningActor.lock();
}