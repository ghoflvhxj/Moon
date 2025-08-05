#pragma once
#include "Include.h"
#include "Core/Object.h"

class Actor;

class ENGINE_DLL Component abstract : public MObject
{
public:
	explicit Component();
	virtual ~Component();

public:
    virtual void Register() {}

public:
	void setOwningActor(std::shared_ptr<Actor> &actor);
	std::shared_ptr<Actor> getOwningActor() const;
private:
	std::weak_ptr<Actor> _pOwningActor;

    REFLECT(Component);
};