#include "Include.h"
#include "DirectionalLightComponent.h"
#include "StaticMeshComponent.h"

#include "Render.h"
#include "Renderer.h"

#include "Material.h"

#include "MainGameSetting.h"

using namespace DirectX;

DirectionalLightComponent::DirectionalLightComponent(void)
	: MLightComponent()
{
	getMesh()->getMaterial(0)->setShader(TEXT("Light.cso"), TEXT("DirectionalLightShader.cso"));
}

DirectionalLightComponent::~DirectionalLightComponent(void)
{

}

void DirectionalLightComponent::Update(const Time deltaTime)
{
    MLightComponent::Update(deltaTime);

    // 빛을 그린다 = 화면을 덮는 평면체 메시를 그린다.
    // 그려질 픽셀의 색상은 난반사, 정반사를 구분해서 결정됨.
    // normal를 샘플링해 난반사 결과를 LightDiffuse에, specular를 샘플링해 정반사 결과를 LightSpecular에 그림
    // 즉, Transform에 이동이나 회전이 들어가면 안된다.
    Vec3 scale = { g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), 1.f };
    Vec3 trans = { 0.f, 0.f, 1.f };
    XMVECTOR scaleVector = XMLoadFloat3(&scale);
    XMVECTOR translationVector = XMLoadFloat3(&trans);
    XMMATRIX IdentityMatrix = XMLoadFloat4x4(&IDENTITYMATRIX);

    XMMATRIX matrices[(int)Transform::End] = {
        XMMatrixScalingFromVector(scaleVector),
        IdentityMatrix,
        XMMatrixTranslationFromVector(translationVector)
    };

    XMStoreFloat4x4(&LightWorldMatrix, matrices[(int)Transform::Scale] * matrices[(int)Transform::Rotation] * matrices[(int)Transform::Translation]);
}

const bool DirectionalLightComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	MLightComponent::GetPrimitiveData(primitiveDataList);

    primitiveDataList[0].PrimitiveType = EPrimitiveType::DirectionalLight;

	return true;
}
