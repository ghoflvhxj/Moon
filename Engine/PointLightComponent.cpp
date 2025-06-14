#include "Include.h"
#include "PointLightComponent.h"

#include "Render.h"

#include "Material.h"

#include "StaticMeshComponent.h"

using namespace DirectX;

PointLightComponent::PointLightComponent(void)
	: MLightComponent()
	, _range{ 1.f }
{
	getMesh()->getMaterial(0)->setShader(TEXT("Light.cso"), TEXT("PointLightShader.cso"));
}

PointLightComponent::~PointLightComponent(void)
{
}

void PointLightComponent::Update(const Time deltaTime)
{
	PrimitiveComponent::Update(deltaTime);

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

const bool PointLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	MLightComponent::GetPrimitiveData(primitiveDataList);
    primitiveDataList[0]._primitiveType = EPrimitiveType::PointLight;

	return true;
}

void PointLightComponent::addRange(float addRange)
{
	_range += addRange;
}

void PointLightComponent::setRange(float range)
{
	_range = range;
}

const float PointLightComponent::getRange() const
{
	return _range;
}
