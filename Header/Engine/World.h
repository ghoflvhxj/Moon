#pragma once
#ifndef __WORLD_H__
#define __WORLD_H__

class Actor;

class ENGINE_DLL World : std::enable_shared_from_this<World>
{
public:
	std::shared_ptr<Actor> SpawnActor();
};

#endif