#include "Include.h"
#include "World.h"

#include "Actor.h"

std::shared_ptr<Actor> World::SpawnActor()
{
	Actor* p = new Actor();
	std::shared_ptr<Actor> p2(p );
	return p2;
}
