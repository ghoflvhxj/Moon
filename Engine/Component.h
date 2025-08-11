#pragma once
#include "Include.h"
#include "Core/Object.h"

class MActor;

class ENGINE_DLL Component abstract : public MObject
{
public:
	explicit Component();
	virtual ~Component();

public:
    virtual void Register() {}

public:
	void setOwningActor(std::shared_ptr<MActor> &actor);
	std::shared_ptr<MActor> getOwningActor() const;
private:
	std::weak_ptr<MActor> _pOwningActor;

    REFLECT(Component);
};