#include "DirectionalLightComponent.h"
#include "StaticMeshComponent.h"

#include "Render.h"
#include "Material.h"
#include "Mesh/StaticMesh/StaticMesh.h"

#include "MainGameSetting.h"

using namespace DirectX;

MDirectionalLightComponent::MDirectionalLightComponent(void)
	: MLightComponent()
{
    Material->setShader(TEXT("Light.cso"), TEXT("DirectionalLightShader.cso"));
}

MDirectionalLightComponent::~MDirectionalLightComponent(void)
{

}

void MDirectionalLightComponent::Update(const Time deltaTime)
{
    MLightComponent::Update(deltaTime);
}

const bool MDirectionalLightComponent::GetPrimitiveData(std::vector<FPrimitiveData>& primitiveDataList)
{
	MLightComponent::GetPrimitiveData(primitiveDataList);

    primitiveDataList[0].PrimitiveType = EPrimitiveType::DirectionalLight;

	return true;
}
