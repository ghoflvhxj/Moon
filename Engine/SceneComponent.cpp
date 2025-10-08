#include "Include.h"
#include "SceneComponent.h"
#include "GraphicDevice.h"

using namespace DirectX;

SceneComponent::SceneComponent()
	: Component()
	, Scale{ FLOAT3_ONE }
	, Rotation{ FLOAT3_ZERO }
	, Translation{ FLOAT3_ZERO }
	, RelativeTranslation{ FLOAT3_ZERO }
	, _worldMatrix(IDENTITYMATRIX)
    , InverseWorldMatrix()
	, _bUpdateable{ false }
	, bUpdated{ false }
{
	setUpdateable(true);
}

SceneComponent::~SceneComponent()
{
}

void SceneComponent::Update(const Time deltaTime)
{
	XMVECTOR vectors[(int)ETransform::End] = {
		XMLoadFloat3(&Scale),
		XMLoadFloat3(&Rotation),
		XMLoadFloat3(&Translation)
	};

	XMMATRIX matrices[(int)ETransform::End] = {
		XMMatrixScalingFromVector(vectors[(int)ETransform::Scale]),
		GetRotationMatrix(),
		XMMatrixTranslationFromVector(vectors[(int)ETransform::Translation])
	};
	
	//XMMatrixMultiply()
	XMStoreFloat4x4(&_worldMatrix, matrices[(int)ETransform::Scale] * matrices[(int)ETransform::Rotation] * matrices[(int)ETransform::Translation]);
    XMStoreFloat4x4(&InverseWorldMatrix, XMMatrixInverse(nullptr, XMLoadFloat4x4(&_worldMatrix)));

	if (ChildComponents.size() > 0)
	{
		XMMATRIX ParentMatrix = XMLoadFloat4x4(&_worldMatrix);
		for (auto ChildComponent : ChildComponents)
		{
			ChildComponent->Update(deltaTime, ParentMatrix);
		}
	}

	bUpdated = true;
}

void SceneComponent::Update(const Time deltaTime, const XMMATRIX& ParentWorldMatrix)
{
	XMVECTOR vectors[(int)ETransform::End] = {
		XMLoadFloat3(&Scale),
		XMLoadFloat3(&Rotation),
		XMLoadFloat3(&RelativeTranslation)
	};

	XMMATRIX matrices[(int)ETransform::End] = {
		XMMatrixScalingFromVector(vectors[(int)ETransform::Scale]),
		GetRotationMatrix(),
		XMMatrixTranslationFromVector(vectors[(int)ETransform::Translation])
	};

	XMStoreFloat4x4(&_worldMatrix, ParentWorldMatrix * matrices[(int)ETransform::Scale] * matrices[(int)ETransform::Rotation] * matrices[(int)ETransform::Translation]);

	if (ChildComponents.size() > 0)
	{
		XMMATRIX ParentMatrix = XMLoadFloat4x4(&_worldMatrix);
		for (auto ChildComponent : ChildComponents)
		{
			ChildComponent->Update(deltaTime, ParentMatrix);
		}
	}

	bUpdated = true;
}

void SceneComponent::OnUpdated()
{
	bUpdated = false;
}

void SceneComponent::setScale(const Vec3& InScale)
{
	Scale = InScale;
}

void SceneComponent::setScale(const float scaleX, const float scaleY, const float scaleZ)
{
	setScale(Vec3{ scaleX, scaleY, scaleZ });
}

void SceneComponent::AddScale(const Vec3& InAdditiveScale)
{
    XMStoreFloat3(&Scale, XMLoadFloat3(&Scale) + XMLoadFloat3(&InAdditiveScale));
}

const Vec3& SceneComponent::getScale() const
{
	return Scale;
}

XMMATRIX SceneComponent::GetRotationMatrix()
{
	return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation));
}

void SceneComponent::setRotation(const Vec3 &rotation)
{
	Rotation = rotation;
}

void SceneComponent::AddRotation(const Vec3& InAdditiveRot)
{
    XMStoreFloat3(&Rotation, XMLoadFloat3(&Rotation) + XMLoadFloat3(&InAdditiveRot));
}

const Vec3& SceneComponent::getRotation() const
{
	return Rotation;
}

void SceneComponent::setTranslation(const Vec3 &translation)
{
	Translation = translation;
}

void SceneComponent::setTranslation(const float transX, const float transY, const float transZ)
{
	setTranslation(Vec3{ transX, transY, transZ });
}

void SceneComponent::AddTranslation(const Vec3& InAdditiveTrans)
{
    XMStoreFloat3(&Translation, XMLoadFloat3(&Translation) + XMLoadFloat3(&InAdditiveTrans));
}

const Vec3& SceneComponent::getTranslation() const
{
	return Translation;
}

const Vec3 SceneComponent::GetForward() const
{
	return { _worldMatrix._31, _worldMatrix._32, _worldMatrix._33 };
}

const Vec3 SceneComponent::getUp() const
{
	return { _worldMatrix._21, _worldMatrix._22, _worldMatrix._23 };
}


const Vec3 SceneComponent::getRight() const
{
	return { _worldMatrix._11, _worldMatrix._12, _worldMatrix._13 };
}

const Vec3 SceneComponent::getWorldTranslation() const
{
	return { _worldMatrix._41, _worldMatrix._42, _worldMatrix._43 };
}

const Mat4 &SceneComponent::getWorldMatrix() const
{
	return _worldMatrix;
}

Mat4& SceneComponent::getWorldMatrix()
{
	return _worldMatrix;
}

void SceneComponent::setUpdateable(const bool updateable)
{
	_bUpdateable = updateable;
}

const bool SceneComponent::isUpdateable() const
{
	return _bUpdateable == true && bUpdated == false;
}

void SceneComponent::AddChildComponent(std::shared_ptr<SceneComponent> Component)
{
	ChildComponents.emplace_back(Component);
}