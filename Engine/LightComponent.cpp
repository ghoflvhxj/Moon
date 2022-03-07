#include "stdafx.h"
#include "LightComponent.h"

#include "Renderer.h"

LightComponent::LightComponent(void)
	: SceneComponent()
	, _color		{ 1.f, 1.f, 1.f }
	, _intensity	{ 1.f }
	, _shown		{ true }
{
}

LightComponent::~LightComponent(void)
{
}

void LightComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);

	g_pRenderer->addLightComponent(shared_from_this());
}

const Vec3& LightComponent::getColor(void) const
{
	return _color;
}

void LightComponent::setColor(const Vec3 &color)
{
	_color = color;
}

void LightComponent::addIntensity(const float addIntensity)
{
	_intensity += addIntensity;
}

void LightComponent::setIntensity(const float intensity)
{
	_intensity = intensity;
}

const float LightComponent::getIntensity()
{
	return _intensity;
}

void LightComponent::show()
{
	_shown = true;
}

void LightComponent::hide()
{
	_shown = false;
}

void LightComponent::toggle()
{
	(true == isShown()) ? hide() : show();
}

const bool LightComponent::isHidden() const
{
	return _shown == false;
}

const bool LightComponent::isShown() const
{
	return _shown == true;
}
