#include "DirectionalLightActor.h"
#include "DirectionalLightComponent.h"
#include "GameFramework/BillboardComponent/BillboardComponent.h"

MDirectionalLightActor::MDirectionalLightActor()
{
    LightComponent = std::make_shared<MDirectionalLightComponent>();
    AddComponent(ROOT_COMPONENT, LightComponent);

    //VisualComponent = std::make_shared<MBillboardComponent>();
    //addComponent(TEXT("Asd"), VisualComponent);
}

