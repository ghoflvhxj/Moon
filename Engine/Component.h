#pragma once
#include "Include.h"
#include "Core/Object.h"
#include "Core/Delegate.h"

class MActor;

class ENGINE_DLL Component abstract : public MObject
{
public:
	explicit Component();
	virtual ~Component();

public:
    virtual void BeginPlay();
    virtual void Register() {}

public:
    FDelegate<void>& GetBeganPlay() { return OnBeganPlayDelegate; }
protected:
    FDelegate<void> OnBeganPlayDelegate;

public:
	void setOwningActor(std::shared_ptr<MActor> &actor);
	std::shared_ptr<MActor>& getOwningActor() const;
private:
	std::weak_ptr<MActor> _pOwningActor;

    REFLECT(Component);
};