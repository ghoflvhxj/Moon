#pragma once

#include "Include.h"
#include "Actor.h"

class DirectionalLightComponent;
class MBillboardComponent;

class ENGINE_DLL MDirectionalLightActor : public MActor
{
public:
    MDirectionalLightActor();

protected:
    std::shared_ptr<DirectionalLightComponent> LightComponent = nullptr;
    std::shared_ptr<MBillboardComponent> VisualComponent = nullptr;

    REFLECT(MDirectionalLightActor)
};