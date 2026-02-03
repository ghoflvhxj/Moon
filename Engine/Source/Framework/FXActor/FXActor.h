#pragma once

#include "Actor.h"

#include "Framework/Component/Billboard/BillboardComponent.h"
#include "Framework/Component/FX/FXComponent.h"

class ENGINE_DLL MFXActor : public MActor
{
public:
    MFXActor();

public:
    virtual void BeginPlay() override;

protected:
    std::shared_ptr<MFXComponent> FXComponent = nullptr;
    std::shared_ptr<MBillboardComponent> VisualComponent = nullptr;

    REFLECT(
        MFXActor
        , PROPERTY(FXComponent)
        //, PROPERTY(VisualComponent)
    )
};