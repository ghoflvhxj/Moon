#include "stdafx.h"
#include "PointLightComponent.h"

#include "Render.h"

#include "Material.h"

#include "StaticMeshComponent.h"

using namespace DirectX;

PointLightComponent::PointLightComponent(void)
	: LightComponent()
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

	XMVECTOR scaleVector = XMLoadFloat3(&getScale());
	XMVECTOR translationVector = XMLoadFloat3(&getTranslation());
	XMMATRIX IdentityMatrix = XMLoadFloat4x4(&IDENTITYMATRIX);
	
	XMMATRIX matrices[(int)Transform::End] = {
		XMMatrixScalingFromVector(scaleVector),
		IdentityMatrix,
		XMMatrixTranslationFromVector(translationVector)
	};

	XMStoreFloat4x4(&getWorldMatrix(), matrices[(int)Transform::Scale] * matrices[(int)Transform::Rotation] * matrices[(int)Transform::Translation]);
}

const bool PointLightComponent::getPrimitiveData(PrimitiveData &primitiveData)
{
	LightComponent::getPrimitiveData(primitiveData);

	primitiveData._pMaterial = getMesh()->getMaterial(0);
	primitiveData._pVertexShader = getMesh()->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = getMesh()->getMaterial(0)->getPixelShader();
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
