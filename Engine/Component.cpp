#include "Include.h"
#include "Component.h"
#include "Actor.h"

#include "World.h"

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

MWorld* MComponent::GetWorld()
{
    assert(getOwningActor());
    if (auto& Owner = getOwningActor()->GetOwner())
    {
        return Owner->CastToShared<MWorld>().get();
    }

    return nullptr;
}
