#include "stdafx.h"
#include "PrimitiveComponent.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"

#include "Renderer.h"

#include "MainGame.h"
#include "MainGameSetting.h"
#include "Camera.h"

PrimitiveComponent::PrimitiveComponent()
	:_eRenderMdoe{ RenderMode::Perspective }
{
}

PrimitiveComponent::~PrimitiveComponent()
{
}

void PrimitiveComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);
	
	g_pRenderer->addPrimitiveComponent(shared_from_this());
}

void PrimitiveComponent::setRenderMode(const RenderMode renderMode)
{
	_eRenderMdoe = renderMode;
}

const PrimitiveComponent::RenderMode PrimitiveComponent::getRenderMdoe() const
{
	return _eRenderMdoe;
}

