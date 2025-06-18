#include "Include.h"
#include "Camera.h"

// Utility
#include "Factory.h"

// Framework
#include "MainGame.h"
#include "MainGameSetting.h"
#include "SceneComponent.h"

using namespace DirectX;

MCamera::MCamera()
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

MCamera::MCamera(const float fov)
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

MCamera::~MCamera()
{
}

void MCamera::initialize()
{
	_pSceneComponent = CreateDefaultSubObject<SceneComponent>();
	addComponent(ROOT_COMPONENT, _pSceneComponent);
	//_pSceneComponent->setTranslation({ 0.f, 0.f, -1.f });

	_pSceneComponent->Update(0.f);
}

void MCamera::tick(const Time deltaTime)
{
	// 이미 업데이트 된 컴포넌트의 정보가 필요함
	_pSceneComponent->Update(deltaTime);

	updateViewMatrix();
	updateProjectionMatrix();
}

void MCamera::updateViewMatrix()
{
	Vec3 eye	= _pSceneComponent->getWorldTranslation();
	Vec3 at		= { 0.f, 0.f, 0.f };
	Vec3 up		= { 0.f, 1.f, 0.f };
	Vec3 to		= _pSceneComponent->GetForward();

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

void MCamera::updateProjectionMatrix()
{
	XMMATRIX matrix = XMMatrixIdentity();

	matrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(getFov()), g_pSetting->getAspectRatio(), Near, Far);
	XMStoreFloat4x4(&_perspectiveProjectionMatrix, matrix);
	matrix = XMMatrixInverse(nullptr, matrix);
	XMStoreFloat4x4(&_inversePerspectiveProjectionMatrix, matrix);

	matrix = XMMatrixOrthographicLH(g_pSetting->getResolutionWidth<float>(), g_pSetting->getResolutionHeight<float>(), Near, Far);
	XMStoreFloat4x4(&_orthographicProjectionMatrix, matrix);
	matrix = XMMatrixInverse(nullptr, matrix);
	XMStoreFloat4x4(&_inverseOrthographicProjectionMatrix, matrix);
}

const Mat4& MCamera::getViewMatrix() const
{
	return _viewMatrix;
}

void MCamera::getViewMatrix(const Mat4 **pMatrix) const
{
	*pMatrix = &_viewMatrix;
}

const Mat4& MCamera::getInvesrViewMatrix() const
{
	return _inverseViewMatrix;
}

const Mat4& MCamera::getProjectionMatrix()
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
		DEV_ASSERT_MSG("알 수 없는 투영모드 입니다!");
		return _perspectiveProjectionMatrix;
	}
	}
}

const Mat4& MCamera::getPerspectiveProjectionMatrix()
{
	return _perspectiveProjectionMatrix;
}

const Mat4& MCamera::getOrthographicProjectionMatrix()
{
	return _orthographicProjectionMatrix;
}

const Mat4& MCamera::getInverseProjectionMatrix()
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
		DEV_ASSERT_MSG("알 수 없는 투영모드 입니다!");
		return _inversePerspectiveProjectionMatrix;
	}
	}
}

const Mat4& MCamera::getInversePerspectiveProjectionMatrix()
{
	return _inversePerspectiveProjectionMatrix;
}

const Mat4& MCamera::getInverseOrthographicProjectionMatrix()
{
	return _inverseOrthographicProjectionMatrix;
}

void MCamera::setLookMode(const MCamera::LookMode lookMode)
{
	_eLookMode = lookMode;
}

const MCamera::LookMode MCamera::getLookMode() const
{
	return _eLookMode;
}

void MCamera::setFov(const float fov)
{
	_fov = fov;
}

const float MCamera::getFov() const
{
	return _fov;
}
