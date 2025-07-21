#include "PointLightActor.h"
#include "PointLightComponent.h"
#include "GameFramework/BillboardComponent/BillboardComponent.h"

MPointLightActor::MPointLightActor()
{
    PointLightComponent = std::make_shared<MPointLightComponent>();
    addComponent(ROOT_COMPONENT, PointLightComponent);

    VisualComponent = std::make_shared<MBillboardComponent>();
    addComponent(TEXT("Asd"), VisualComponent);
}

