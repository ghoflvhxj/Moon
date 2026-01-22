#include "LightComponent.h"

#include "MoonEngine.h"
#include "World.h"
#include "Window.h"
#include "Renderer.h"
#include "MainGameSetting.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Core/ResourceManager.h"
#include "Actor.h"

using namespace DirectX;

MLightComponent::MLightComponent(void)
	: MPrimitiveComponent()
	, Color	{ 1.f, 1.f, 1.f }
	, Intensity { 1.f }
	, bShow { true }
{
    Material = std::make_shared<MMaterial>();
}

MLightComponent::~MLightComponent(void)
{
}

void MLightComponent::OnLoaded()
{
    Super::OnLoaded();
}

void MLightComponent::Update(const Time deltaTime)
{
    Super::Update(deltaTime);

    TransformMatrix(LightWorldMatrix, getScale(), VEC3ZERO, VEC3ZERO);

    Direction = GetForward();
}

const bool MLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	FPrimitiveData primitiveData        = {};
	primitiveData.PrimitiveComponent	= GetShared();
    primitiveData.Material              = Material;

	primitiveDataList.emplace_back(primitiveData);

	return true;
}

Mat4& MLightComponent::getWorldMatrix()
{
    return LightWorldMatrix;
}

const Vec3& MLightComponent::getColor(void) const
{
	return Color;
}

void MLightComponent::setColor(const Vec3 &color)
{
	Color = color;
}

void MLightComponent::addIntensity(const float addIntensity)
{
	Intensity += addIntensity;
}

void MLightComponent::setIntensity(const float intensity)
{
	Intensity = intensity;
}

const float MLightComponent::getIntensity()
{
	return Intensity;
}

void MLightComponent::show()
{
	bShow = true;
}

void MLightComponent::hide()
{
	bShow = false;
}

void MLightComponent::toggle()
{
	(true == isShown()) ? hide() : show();
}

const bool MLightComponent::isHidden() const
{
	return bShow == false;
}

const bool MLightComponent::isShown() const
{
	return bShow == true;
}
