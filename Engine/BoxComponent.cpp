#include "stdafx.h"
#include "BoxComponent.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "TextureComponent.h"

BoxComponent::BoxComponent()
	: PrimitiveComponent()
	, _color{ 1.f, 1.f, 1.f, 1.f }
{
}

BoxComponent::~BoxComponent()
{
}

void BoxComponent::render()
{
}

void BoxComponent::setMaterial(std::shared_ptr<Material> pMaterial)
{
	_pMaterial = pMaterial;
}

std::shared_ptr<Material> &BoxComponent::getMaterial()
{
	return _pMaterial;
}

void BoxComponent::setColor(const Vec4 &color)
{
	_color = color;
}

const Vec4 &BoxComponent::getColor(void)
{
	return _color;
}
