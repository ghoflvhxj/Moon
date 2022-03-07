#include "stdafx.h"
#include "SkyComponent.h"

#include "ShaderManager.h"

#include "StaticMeshComponent.h"
#include "TextureComponent.h"
#include "Material.h"
#include "Renderer.h"

using namespace DirectX;

SkyComponent::SkyComponent()
	: SceneComponent()
	, _baseColor{ 1.f, 1.f, 1.f }
{
	initialize();
}

SkyComponent::~SkyComponent()
{
}

void SkyComponent::initialize()
{
	_pStaticMeshComponent = std::make_shared<StaticMeshComponent>("SkyDome/SkyDome.fbx");
	//addComponent(TEXT("test3"), _pStaticMeshComponent3);
	//_pStaticMeshComponent->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
	_pStaticMeshComponent->setRotation(Vec3{ XMConvertToRadians(-90.f), XMConvertToRadians(90.f), 0.f });
	_pStaticMeshComponent->getMaterial(0)->setCullMode(Graphic::CullMode::None);
	//_pStaticMeshComponent->getMaterial(0)->setDepthWriteMode(DepthWriteMode::Disable);

	_pStaticMeshComponent->getMaterial(0)->setShader(TEXT("TexVertexShader.cso"), TEXT("SkyPixelShader.cso"));
	_pStaticMeshComponent->Update(0.f);
}

void SkyComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);
	g_pRenderer->addSkyComponent(shared_from_this());
}

void SkyComponent::render()
{
	_pStaticMeshComponent->render();
}

void SkyComponent::Test(ID3D11ShaderResourceView *p)
{
	_pStaticMeshComponent->getMaterial(0)->setTexture(int32ToEnum<TextureType>(1), std::make_shared<TextureComponent>(p));
}


