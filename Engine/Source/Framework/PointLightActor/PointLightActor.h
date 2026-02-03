#pragma once

#include "Include.h"
#include "Actor.h"

class MPointLightComponent;
class MBillboardComponent;

class ENGINE_DLL MPointLightActor : public MActor
{
public:
    MPointLightActor();

public:
    virtual void BeginPlay() override;

public:
    std::shared_ptr<MPointLightComponent> GetPointLightComponent() const { return PointLightComponent; }
protected:
    std::shared_ptr<MPointLightComponent> PointLightComponent = nullptr;
    std::shared_ptr<MBillboardComponent> VisualComponent = nullptr;

    REFLECT(
        MPointLightActor
        , PROPERTY(PointLightComponent)
        //, PROPERTY(VisualComponent)
    )
};