#pragma once
#include "Include.h"
#include "Core/Object.h"
#include "Core/Delegate.h"

class MActor;
class MWorld;

class ENGINE_DLL MComponent abstract : public MObject
{
public:
	explicit MComponent();
	virtual ~MComponent();

public:
    virtual void BeginPlay();

public:
    virtual void OnRegisted();
    FDelegate<void>& GetOnRegistedDelegate();
protected:
    FDelegate<void> OnRegistedDelegate;

public:
    FDelegate<void>& GetBeganPlay() { return OnBeganPlayDelegate; }
protected:
    FDelegate<void> OnBeganPlayDelegate; 

public:
	void setOwningActor(std::shared_ptr<MActor> &actor);
	std::shared_ptr<MActor> getOwningActor() const;
private:
	std::weak_ptr<MActor> _pOwningActor;

public:
    void SetName(const std::wstring& InName) { Name = InName; }
    const std::wstring& GetName() const { return Name; }
protected:
    std::wstring Name;

public:
    MWorld* GetWorld();

    REFLECT(
        MComponent
        , PROPERTY(Name)
    );
};