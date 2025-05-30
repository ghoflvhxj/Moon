#include "Include.h"
#include "DirectionalLightComponent.h"
#include "StaticMeshComponent.h"

#include "Render.h"
#include "Renderer.h"

#include "Material.h"

#include "MainGameSetting.h"

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

	_forward = getLook();

	// 빛이 아닌 평면 메시를 그리기 위해서 사용되니 수정해주는 부분...!
	XMFLOAT3 scale = { CastValue<float>(g_pSetting->getResolutionWidth()), CastValue<float>(g_pSetting->getResolutionHeight()), 1.f };
	XMFLOAT3 trans = { 0.f, 0.f, 1.f };
	XMVECTOR scaleVector = XMLoadFloat3(&scale);
	XMVECTOR translationVector = XMLoadFloat3(&trans);
	XMMATRIX IdentityMatrix = XMLoadFloat4x4(&IDENTITYMATRIX);

	XMMATRIX matrices[(int)Transform::End] = {
		XMMatrixScalingFromVector(scaleVector),
		IdentityMatrix,
		XMMatrixTranslationFromVector(translationVector)
	};

	XMStoreFloat4x4(&getWorldMatrix(), matrices[(int)Transform::Scale] * matrices[(int)Transform::Rotation] * matrices[(int)Transform::Translation]);
}

const bool DirectionalLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	LightComponent::GetPrimitiveData(primitiveDataList);

	primitiveDataList[0]._pMaterial = getMesh()->getMaterials()[0];

	g_pRenderer->addDirectionalLightInfoForShadow(_forward);

	return true;
}
