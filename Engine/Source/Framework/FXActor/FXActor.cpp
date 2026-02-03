#include "FXActor.h"
#include "Core/ResourceManager.h"
#include "World.h"

MFXActor::MFXActor()
{
    VisualComponent = std::make_shared<MBillboardComponent>();
    AddComponent(TEXT("VisualHelper"), VisualComponent);
    VisualComponent->SetWorldRotation();
    VisualComponent->SetMesh(TEXT("Base/Plane.json"));
    VisualComponent->SetPrimitiveType(EPrimitiveType::CustomPrimitiveType0);
    VisualComponent->bAlwaysUpdate = true;

    std::shared_ptr<MTexture> Texture = nullptr;
    g_ResourceManager->Load(TEXT("Resources/Texture/FX.png"), Texture);
    VisualComponent->Material->setTexture(ETextureType::Diffuse, Texture);
    VisualComponent->Material->setShader(TEXT("VS_SimpleTexture.cso"), TEXT("PS_SimpleTexture.cso"));
    VisualComponent->Material->SetAlphaMask(true);

    FXComponent = std::make_shared<MFXComponent>();
    AddComponent(ROOT_COMPONENT, FXComponent);

    FXComponent->AddChildComponent(VisualComponent);
}

void MFXActor::BeginPlay()
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
