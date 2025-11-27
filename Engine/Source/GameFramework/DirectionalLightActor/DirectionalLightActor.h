#pragma once

#include "Include.h"
#include "Actor.h"

#include "DirectionalLightComponent.h"

class MDirectionalLightComponent;
class MBillboardComponent;

class ENGINE_DLL MDirectionalLightActor : public MActor
{
public:
    MDirectionalLightActor();

protected:
    std::shared_ptr<MDirectionalLightComponent> LightComponent = nullptr;
    std::shared_ptr<MBillboardComponent> VisualComponent = nullptr;

    REFLECT(
        MDirectionalLightActor
        , PROPERTY(LightComponent)
    )
};