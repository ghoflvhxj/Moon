#pragma once
#ifndef __CAMERA_H__

#include "Actor.h"

class SceneComponent;

class ENGINE_DLL Camera : public Actor
{
public:
	enum class ProjectMode
	{
		Perspective, Orthograhpic, End
	};

	enum class LookMode
	{
		At, To, End
	};

public:
	explicit Camera();
	explicit Camera(const float fov);
	virtual ~Camera();
private:
	void initialize();

public:
	virtual void tick(const Time deltaTime) override;
private:
	void updateViewMatrix();
	void updateProjectionMatrix();

public:
	const Mat4 &getViewMatrix() const;
	void getViewMatrix(const Mat4 **pMatrix) const;
private:
	Mat4 _viewMatrix;
public:
	const Mat4& getInvesrViewMatrix() const;
	Mat4 _inverseViewMatrix;

public:
	const Mat4& getProjectionMatrix();
	const Mat4& getPerspectiveProjectionMatrix();
	const Mat4& getOrthographicProjectionMatrix();
private:
	Mat4 _perspectiveProjectionMatrix;
	Mat4 _orthographicProjectionMatrix;

public:
	const Mat4& getInverseProjectionMatrix();
	const Mat4& getInversePerspectiveProjectionMatrix();
	const Mat4& getInverseOrthographicProjectionMatrix();
private:
	Mat4 _inversePerspectiveProjectionMatrix;
	Mat4 _inverseOrthographicProjectionMatrix;

private:
	ProjectMode _eProjection;

public:
	void setLookMode(const LookMode lookMode);
	const LookMode getLookMode() const;
private:
	LookMode _eLookMode;

public:
	void setFov(const float fov);
	const float getFov() const;
private:
	float _fov;

private:
	std::shared_ptr<SceneComponent> _pSceneComponent;
};

#define __CAMERA_H__
#endif