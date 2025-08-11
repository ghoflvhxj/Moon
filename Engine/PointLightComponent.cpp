#include "PointLightComponent.h"

#include "Render.h"
#include "Material.h"

#include "Mesh/StaticMesh/StaticMesh.h"

using namespace DirectX;

MPointLightComponent::MPointLightComponent(void)
	: MLightComponent()
	, Range{ 1.f }
{
    Material->setShader(TEXT("Light.cso"), TEXT("PointLightShader.cso"));
}

MPointLightComponent::~MPointLightComponent(void)
{
}

void MPointLightComponent::Update(const Time deltaTime)
{
	Super::Update(deltaTime);

	Vec3 trans = { 0.f, 0.f, 1.f };

	// 회전을 제거한 월드행렬 만들기
	XMVECTOR scaleVector = XMLoadFloat3(&getScale());
	XMVECTOR translationVector = XMLoadFloat3(&trans);
	XMMATRIX IdentityMatrix = XMLoadFloat4x4(&IDENTITYMATRIX);
	
	XMMATRIX matrices[(int)Transform::End] = {
		XMMatrixScalingFromVector(scaleVector),
		IdentityMatrix,
		XMMatrixTranslationFromVector(translationVector)
	};

	XMStoreFloat4x4(&LightWorldMatrix, matrices[(int)Transform::Scale] * matrices[(int)Transform::Rotation] * matrices[(int)Transform::Translation]);
}

const bool MPointLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	MLightComponent::GetPrimitiveData(primitiveDataList);
    primitiveDataList[0].PrimitiveType = EPrimitiveType::PointLight;

	return true;
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
