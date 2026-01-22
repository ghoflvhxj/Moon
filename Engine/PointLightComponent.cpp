#include "PointLightComponent.h"

#include "Render.h"
#include "Material.h"

#include "Mesh/StaticMesh/StaticMesh.h"

using namespace DirectX;

MPointLightComponent::MPointLightComponent()
	: MLightComponent()
	, Range{ 1.f }
{
    Material->setShader(TEXT("Light.cso"), TEXT("PointLightShader.cso"));
}

MPointLightComponent::~MPointLightComponent()
{
}

void MPointLightComponent::Update(const Time deltaTime)
{
	Super::Update(deltaTime);
}

const bool MPointLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
    if (Super::GetPrimitiveData(primitiveDataList))
    {
        primitiveDataList[0].PrimitiveType = EPrimitiveType::PointLight;
	    return true;
    }

    return false;
}

void MPointLightComponent::addRange(float addRange)
{
	Range += addRange;
}

void MPointLightComponent::setRange(float range)
{
	Range = range;
}

const float MPointLightComponent::getRange() const
{
	return Range;
}
