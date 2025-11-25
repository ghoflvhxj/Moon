#include "Include.h"
#include "SceneComponent.h"
#include "GraphicDevice.h"

using namespace DirectX;

MSceneComponent::MSceneComponent()
	: MComponent()
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

MSceneComponent::~MSceneComponent()
{
}

void MSceneComponent::Update(const Time deltaTime)
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

void MSceneComponent::Update(const Time deltaTime, const XMMATRIX& ParentWorldMatrix)
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

void MSceneComponent::OnUpdated()
{
	bUpdated = false;
}

void MSceneComponent::setScale(const Vec3& InScale)
{
	Scale = InScale;
}

void MSceneComponent::setScale(const float scaleX, const float scaleY, const float scaleZ)
{
	setScale(Vec3{ scaleX, scaleY, scaleZ });
}

void MSceneComponent::AddScale(const Vec3& InAdditiveScale)
{
    XMStoreFloat3(&Scale, XMLoadFloat3(&Scale) + XMLoadFloat3(&InAdditiveScale));
}

const Vec3& MSceneComponent::getScale() const
{
	return Scale;
}

XMMATRIX MSceneComponent::GetRotationMatrix()
{
	return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation));
}

void MSceneComponent::setRotation(const Vec3 &rotation)
{
	Rotation = rotation;
}

void MSceneComponent::AddRotation(const Vec3& InAdditiveRot)
{
    XMStoreFloat3(&Rotation, XMLoadFloat3(&Rotation) + XMLoadFloat3(&InAdditiveRot));
}

const Vec3& MSceneComponent::getRotation() const
{
	return Rotation;
}

void MSceneComponent::setTranslation(const Vec3 &translation)
{
	Translation = translation;
}

void MSceneComponent::setTranslation(const float transX, const float transY, const float transZ)
{
	setTranslation(Vec3{ transX, transY, transZ });
}

void MSceneComponent::AddTranslation(const Vec3& InAdditiveTrans)
{
    XMStoreFloat3(&Translation, XMLoadFloat3(&Translation) + XMLoadFloat3(&InAdditiveTrans));
}

const Vec3& MSceneComponent::getTranslation() const
{
	return Translation;
}

const Vec3 MSceneComponent::GetForward() const
{
	return { _worldMatrix._31, _worldMatrix._32, _worldMatrix._33 };
}

const Vec3 MSceneComponent::getUp() const
{
	return { _worldMatrix._21, _worldMatrix._22, _worldMatrix._23 };
}


const Vec3 MSceneComponent::getRight() const
{
	return { _worldMatrix._11, _worldMatrix._12, _worldMatrix._13 };
}

const Vec3 MSceneComponent::getWorldTranslation() const
{
	return { _worldMatrix._41, _worldMatrix._42, _worldMatrix._43 };
}

const Mat4 &MSceneComponent::getWorldMatrix() const
{
	return _worldMatrix;
}

Mat4& MSceneComponent::getWorldMatrix()
{
	return _worldMatrix;
}

void MSceneComponent::setUpdateable(const bool updateable)
{
	_bUpdateable = updateable;
}

const bool MSceneComponent::isUpdateable() const
{
	return _bUpdateable == true && bUpdated == false;
}

void MSceneComponent::AddChildComponent(std::shared_ptr<MSceneComponent> Component)
{
	ChildComponents.emplace_back(Component);
}