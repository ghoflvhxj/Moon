#pragma once

#include "Include.h"
#include "Actor.h"

class MPointLightComponent;
class MBillboardComponent;

class ENGINE_DLL MPointLightActor : public Actor
{
public:
    MPointLightActor();

protected:
    std::shared_ptr<MPointLightComponent> PointLightComponent = nullptr;
    std::shared_ptr<MBillboardComponent> VisualComponent = nullptr;
};