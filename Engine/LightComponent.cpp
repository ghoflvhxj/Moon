#include "LightComponent.h"

#include "Renderer.h"
#include "MainGameSetting.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Core/ResourceManager.h"

using namespace DirectX;

MLightComponent::MLightComponent(void)
	: MPrimitiveComponent()
	, Color	{ 1.f, 1.f, 1.f }
	, Intensity { 1.f }
	, bShow { true }
{
    g_ResourceManager->Load(TEXT("Base/Plane.json"), _pStaticMesh);

    Material = std::make_shared<MMaterial>();

	setScale(g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 1.f);
	setTranslation(0.f, 0.f, 1.f);
}

MLightComponent::~MLightComponent(void)
{
}

void MLightComponent::Update(const Time deltaTime)
{
    Super::Update(deltaTime);

    Direction = GetForward();
}

const bool MLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	FPrimitiveData primitiveData        = {};
	primitiveData.PrimitiveComponent	= GetShared();
	primitiveData.MeshData              = &_pStaticMesh->GetMeshData(0);
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
