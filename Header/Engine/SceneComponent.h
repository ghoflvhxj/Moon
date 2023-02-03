#pragma once
#ifndef __SCENE_COMPONENT_H__

#include "Component.h"

class MainGame;

class ENGINE_DLL SceneComponent : public Component
{
public:
	enum class Transform { Scale, Rotation, Translation, End };

public:
	explicit SceneComponent();
	virtual ~SceneComponent();

public:
	virtual void Update(const Time deltaTime);
	virtual void Update(const Time deltaTime, const XMMATRIX& ParentWorldMatrix);
	virtual void OnUpdated();

public:
	virtual void setScale(const Vec3 &scale);
	void setScale(const float scaleX, const float scaleY, const float scaleZ);
	const Vec3& getScale() const;
private:
	Vec3 m_scale;

public:
	virtual XMMATRIX GetRotationMatrix();
	void setRotation(const Vec3 &rotation);
	const Vec3&	getRotation() const;
private:
	Vec3 m_rotation;

public:
	virtual void setTranslation(const Vec3 &translation);
	void setTranslation(const float transX, const float transY, const float transZ);
	const Vec3&	getTranslation() const;
private:
	Vec3 m_translation;
	Vec3 RelativeTranslation;
public:
	const Vec3			getLook() const;
	const Vec3			getUp() const;
	const Vec3			getRight() const;
	const Vec3			getWorldTranslation() const;
	const Mat4&			getWorldMatrix() const;
	Mat4&				getWorldMatrix();
private:
	Mat4 _worldMatrix;

public:
	void		setUpdateable(const bool isUpdateable);
	const bool	isUpdateable() const;
private:
	bool _bUpdateable;
	bool bUpdated;

public:
	void AddChildComponent(std::shared_ptr<SceneComponent> Component);
private:
	std::vector<std::shared_ptr<SceneComponent>> ChildComponents;
};

#define __SCENE_COMPONENT_H__
#endif