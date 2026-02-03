#include "DirectionalLightActor.h"

#include "World.h"

#include "Core/ResourceManager.h"
#include "Texture.h"

MDirectionalLightActor::MDirectionalLightActor()
{
    VisualComponent = std::make_shared<MBillboardComponent>();
    AddComponent(TEXT("VisualHelper"), VisualComponent);
    VisualComponent->SetWorldRotation();
    VisualComponent->SetMesh(TEXT("Base/Plane.json"));
    VisualComponent->SetPrimitiveType(EPrimitiveType::CustomPrimitiveType0);
    VisualComponent->bAlwaysUpdate = true;

    std::shared_ptr<MTexture> Texture = nullptr;
    g_ResourceManager->Load(TEXT("Resources/Texture/PointLight.png"), Texture);
    VisualComponent->Material->setTexture(ETextureType::Diffuse, Texture);
    VisualComponent->Material->setShader(TEXT("VS_SimpleTexture.cso"), TEXT("PS_SimpleTexture.cso"));
    VisualComponent->Material->SetAlphaMask(true);

    LightComponent = std::make_shared<MDirectionalLightComponent>();
    AddComponent(ROOT_COMPONENT, LightComponent);

    LightComponent->AddChildComponent(VisualComponent);
}

void MDirectionalLightActor::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld()->IsWorldType(EWorldType::Editor))
    {

    }
    else
    {
        VisualComponent->SetRendering(false);
    }
}

