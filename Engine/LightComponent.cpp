#include "Include.h"
#include "LightComponent.h"

#include "Renderer.h"
#include "MainGameSetting.h"
#include "StaticMeshComponent.h"

LightComponent::LightComponent(void)
	: PrimitiveComponent()
	, _color		{ 1.f, 1.f, 1.f }
	, _intensity	{ 1.f }
	, _shown		{ true }
{
	_pStaticMesh = std::make_shared<StaticMesh>();
	_pStaticMesh->InitializeFromFBX(TEXT("Base/Plane.fbx"));

	if (_pStaticMesh->getMaterialCount() == 0)
	{
		
	}

	setScale(CastValue<float>(g_pSetting->getResolutionWidth()), CastValue<float>(g_pSetting->getResolutionHeight()), 1.f);
	setTranslation(0.f, 0.f, 1.f);
}

LightComponent::~LightComponent(void)
{
}

const bool LightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	FPrimitiveData primitiveData = {};
	primitiveData._pPrimitive		= shared_from_this();
	primitiveData.MeshData = _pStaticMesh->GetMeshData(0);
	primitiveData._primitiveType	= EPrimitiveType::Light;

	primitiveDataList.emplace_back(primitiveData);

	return true;
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
