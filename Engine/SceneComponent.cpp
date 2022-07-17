#include "stdafx.h"
#include "SceneComponent.h"
#include "GraphicDevice.h"

using namespace DirectX;

SceneComponent::SceneComponent()
	: Component()
	, m_scale{ FLOAT3_ONE }
	, m_rotation{ FLOAT3_ZERO }
	, m_translation{ FLOAT3_ZERO }
	, _worldMatrix()
{
	setUpdateable(true);
}

SceneComponent::~SceneComponent()
{
}

void SceneComponent::Update(const Time deltaTime)
{
	XMVECTOR vectors[(int)Transform::End] = {
		XMLoadFloat3(&m_scale),
		XMLoadFloat3(&m_rotation),
		XMLoadFloat3(&m_translation)
	};

	XMMATRIX matrices[(int)Transform::End] = {
		XMMatrixScalingFromVector(vectors[(int)Transform::Scale]),
		XMMatrixRotationRollPitchYawFromVector(vectors[(int)Transform::Rotation]),
		XMMatrixTranslationFromVector(vectors[(int)Transform::Translation])
	};
	
	//XMMatrixMultiply()
	XMStoreFloat4x4(&_worldMatrix, matrices[(int)Transform::Scale] * matrices[(int)Transform::Rotation] * matrices[(int)Transform::Translation]);
}

void SceneComponent::setScale(const Vec3 &scale)
{
	m_scale = scale;
}

void SceneComponent::setScale(const float scaleX, const float scaleY, const float scaleZ)
{
	m_scale = { scaleX, scaleY, scaleZ };
}

const Vec3& SceneComponent::getScale() const
{
	return m_scale;
}

void SceneComponent::setRotation(const Vec3 &rotation)
{
	m_rotation = rotation;
}

const Vec3& SceneComponent::getRotation() const
{
	return m_rotation;
}

void SceneComponent::setTranslation(const Vec3 &translation)
{
	m_translation = translation;
}

void SceneComponent::setTranslation(const float transX, const float transY, const float transZ)
{
	m_translation = { transX, transY, transZ };
}

const Vec3& SceneComponent::getTranslation() const
{
	return m_translation;
}

const Vec3 SceneComponent::getLook() const
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
	return _bUpdateable;
}
