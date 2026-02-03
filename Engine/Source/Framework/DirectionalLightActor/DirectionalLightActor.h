#pragma once

#include "Include.h"
#include "Actor.h"

#include "DirectionalLightComponent.h"
#include "Framework/Component/Billboard/BillboardComponent.h"

class ENGINE_DLL MDirectionalLightActor : public MActor
{
public:
    MDirectionalLightActor();

public:
    virtual void BeginPlay() override;

protected:
    std::shared_ptr<MDirectionalLightComponent> LightComponent = nullptr;
    std::shared_ptr<MBillboardComponent> VisualComponent = nullptr;

    REFLECT(
        MDirectionalLightActor
        , PROPERTY(LightComponent)
        //, PROPERTY(VisualComponent)
    )
};