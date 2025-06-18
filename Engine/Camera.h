#pragma once
#ifndef __CAMERA_H__

#include "Actor.h"

class SceneComponent;

class ENGINE_DLL MCamera : public Actor
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
	explicit MCamera();
	explicit MCamera(const float fov);
	virtual ~MCamera();
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

public:
    float GetNear() const { return Near; }
    float GetFar() const { return Far; }
private:
    float Near = 0.1f;
    float Far = 1000.f;

private:
	std::shared_ptr<SceneComponent> _pSceneComponent;
};

#define __CAMERA_H__
#endif