#include "LightComponent.h"

#include "Renderer.h"
#include "MainGameSetting.h"
#include "Mesh/StaticMesh/StaticMesh.h"

using namespace DirectX;

MLightComponent::MLightComponent(void)
	: MPrimitiveComponent()
    , Direction     {VEC3ZERO}
	, _color		{ 1.f, 1.f, 1.f }
	, _intensity	{ 1.f }
	, _shown		{ true }
{
	_pStaticMesh = std::make_shared<StaticMesh>();
	_pStaticMesh->LoadFromFBX(TEXT("Base/Plane.fbx"));

	if (_pStaticMesh->GetMaterialNum() == 0)
	{
		
	}

	setScale(g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 1.f);
	setTranslation(0.f, 0.f, 1.f);
}

MLightComponent::~MLightComponent(void)
{
}

void MLightComponent::Update(const Time deltaTime)
{
    MPrimitiveComponent::Update(deltaTime);

    Direction = GetForward();
}

const bool MLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	FPrimitiveData primitiveData        = {};
	primitiveData.PrimitiveComponent	= shared_from_this();
	primitiveData.MeshData              = _pStaticMesh->GetMeshData(0);
    primitiveData.Material              = getMesh()->getMaterials()[0];

	primitiveDataList.emplace_back(primitiveData);

	return true;
}

Mat4& MLightComponent::getWorldMatrix()
{
    return LightWorldMatrix;
}

const Vec3& MLightComponent::getColor(void) const
{
	return _color;
}

void MLightComponent::setColor(const Vec3 &color)
{
	_color = color;
}

void MLightComponent::addIntensity(const float addIntensity)
{
	_intensity += addIntensity;
}

void MLightComponent::setIntensity(const float intensity)
{
	_intensity = intensity;
}

const float MLightComponent::getIntensity()
{
	return _intensity;
}

void MLightComponent::show()
{
	_shown = true;
}

void MLightComponent::hide()
{
	_shown = false;
}

void MLightComponent::toggle()
{
	(true == isShown()) ? hide() : show();
}

const bool MLightComponent::isHidden() const
{
	return _shown == false;
}

const bool MLightComponent::isShown() const
{
	return _shown == true;
}
