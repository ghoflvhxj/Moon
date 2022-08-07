#include "stdafx.h"
#include "DirectionalLightComponent.h"

#include "Render.h"

#include "Material.h"

#include "StaticMeshComponent.h"

using namespace DirectX;

DirectionalLightComponent::DirectionalLightComponent(void)
	: LightComponent()
{
	getMesh()->getMaterial(0)->setShader(TEXT("Light.cso"), TEXT("DirectionalLightShader.cso"));
}

DirectionalLightComponent::~DirectionalLightComponent(void)
{

}

void DirectionalLightComponent::Update(const Time deltaTime)
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

const bool DirectionalLightComponent::getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList)
{
	LightComponent::getPrimitiveData(primitiveDataList);

	primitiveDataList[0]._pMaterial = getMesh()->getMaterials()[0];
	primitiveDataList[0]._pVertexShader = getMesh()->getMaterial(0)->getVertexShader();
	primitiveDataList[0]._pPixelShader = getMesh()->getMaterial(0)->getPixelShader();

	return true;
}
