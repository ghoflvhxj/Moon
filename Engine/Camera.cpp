#include "stdafx.h"
#include "Camera.h"

// Utility
#include "Factory.h"

// Framework
#include "MainGame.h"
#include "MainGameSetting.h"
#include "SceneComponent.h"

using namespace DirectX;

Camera::Camera()
	: Actor()
	, _viewMatrix()
	, _perspectiveProjectionMatrix()
	, _eProjection{ ProjectMode::Perspective }
	, _eLookMode{ LookMode::At }
	, _fov{ g_pSetting->getFov() }
	, _pSceneComponent{ nullptr }
{
	initialize();
}

Camera::Camera(const float fov)
	: Actor()
	, _viewMatrix()
	, _perspectiveProjectionMatrix()
	, _eProjection{ ProjectMode::Perspective }
	, _eLookMode{ LookMode::At }
	, _fov{ fov }
	, _pSceneComponent{ nullptr }
{
	initialize();
}

Camera::~Camera()
{
}

void Camera::initialize()
{
	_pSceneComponent = CreateDefaultSubObject<SceneComponent>();
	addComponent(ROOT_COMPONENT, _pSceneComponent);
	_pSceneComponent->setTranslation({ 0.f, 0.f, -1.f });

	_pSceneComponent->Update(0.f);
}

void Camera::tick(const Time deltaTime)
{
	// �̹� ������Ʈ �� ������Ʈ�� ������ �ʿ���
	_pSceneComponent->Update(deltaTime);

	updateViewMatrix();
	updateProjectionMatrix();
}

void Camera::updateViewMatrix()
{
	const Mat4 &worldMatrix = _pSceneComponent->getWorldMatrix();
	Vec3 eye	= _pSceneComponent->getWorldTranslation();
	Vec3 at		= { 0.f, 0.f, 0.f };
	Vec3 up		= { 0.f, 1.f, 0.f };
	Vec3 to		= _pSceneComponent->getLook();

	XMMATRIX viewMatrix = XMMatrixIdentity();
	switch (_eLookMode)
	{
	case LookMode::At:
		viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up));
		break;
	case LookMode::To:
		viewMatrix = XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&to), XMLoadFloat3(&up));
		break;
	}

	XMStoreFloat4x4(&_viewMatrix, viewMatrix);

	viewMatrix = XMMatrixInverse(nullptr, viewMatrix);
	XMStoreFloat4x4(&_inverseViewMatrix, viewMatrix);
}

void Camera::updateProjectionMatrix()
{
	XMMATRIX matrix = XMMatrixIdentity();

	matrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(getFov()), g_pSetting->getAspectRatio(), 0.1f, 1000.f);
	XMStoreFloat4x4(&_perspectiveProjectionMatrix, matrix);
	matrix = XMMatrixInverse(nullptr, matrix);
	XMStoreFloat4x4(&_inversePerspectiveProjectionMatrix, matrix);

	matrix = XMMatrixOrthographicLH(Int32ToFloat(g_pSetting->getResolutionWidth()), Int32ToFloat(g_pSetting->getResolutionHeight()), 0.1f, 1000.f);
	XMStoreFloat4x4(&_orthographicProjectionMatrix, matrix);
	matrix = XMMatrixInverse(nullptr, matrix);
	XMStoreFloat4x4(&_inverseOrthographicProjectionMatrix, matrix);
}

const Mat4& Camera::getViewMatrix() const
{
	return _viewMatrix;
}

void Camera::getViewMatrix(const Mat4 **pMatrix) const
{
	*pMatrix = &_viewMatrix;
}

const Mat4& Camera::getInvesrViewMatrix() const
{
	return _inverseViewMatrix;
}

const Mat4& Camera::getProjectionMatrix()
{
	switch (_eProjection)
	{
	case ProjectMode::Perspective:
	{
		return _perspectiveProjectionMatrix;
	}
	case ProjectMode::Orthograhpic:
	{
		return _orthographicProjectionMatrix;
	}
	default:
	{
		DEV_ASSERT_MSG(TEXT("�� �� ���� ������� �Դϴ�!"));
		return _perspectiveProjectionMatrix;
	}
	}
}

const Mat4& Camera::getPerspectiveProjectionMatrix()
{
	return _perspectiveProjectionMatrix;
}

const Mat4& Camera::getOrthographicProjectionMatrix()
{
	return _orthographicProjectionMatrix;
}

const Mat4& Camera::getInverseProjectionMatrix()
{
	switch (_eProjection)
	{
	case ProjectMode::Perspective:
	{
		return _inversePerspectiveProjectionMatrix;
	}
	case ProjectMode::Orthograhpic:
	{
		return _inverseOrthographicProjectionMatrix;
	}
	default:
	{
		DEV_ASSERT_MSG(TEXT("�� �� ���� ������� �Դϴ�!"));
		return _inversePerspectiveProjectionMatrix;
	}
	}
}

const Mat4& Camera::getInversePerspectiveProjectionMatrix()
{
	return _inversePerspectiveProjectionMatrix;
}

const Mat4& Camera::getInverseOrthographicProjectionMatrix()
{
	return _inverseOrthographicProjectionMatrix;
}

void Camera::setLookMode(const Camera::LookMode lookMode)
{
	_eLookMode = lookMode;
}

const Camera::LookMode Camera::getLookMode() const
{
	return _eLookMode;
}

void Camera::setFov(const float fov)
{
	_fov = fov;
}

const float Camera::getFov() const
{
	return _fov;
}
