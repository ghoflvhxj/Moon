#pragma once
#include "Include.h"

class Actor;

class ENGINE_DLL Component abstract
{
public:
	explicit Component();
	virtual ~Component();

public:
	void setOwningActor(std::shared_ptr<Actor> &actor);
	std::shared_ptr<Actor> getOwningActor() const;
private:
	std::weak_ptr<Actor> _pOwningActor;
};