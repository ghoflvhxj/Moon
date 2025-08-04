#pragma once

#include "Component.h"

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
	virtual void setScale(const Vec3& InScale);
	void setScale(const float scaleX, const float scaleY, const float scaleZ);
    void AddScale(const Vec3& InAdditiveScale);
	const Vec3& getScale() const;
private:
	Vec3 Scale;

public:
	virtual XMMATRIX GetRotationMatrix();
	void setRotation(const Vec3 &rotation);
    void AddRotation(const Vec3& InAdditiveRot);
	const Vec3&	getRotation() const;
private:
	Vec3 Rotation;

public:
	virtual void setTranslation(const Vec3 &translation);
	void setTranslation(const float transX, const float transY, const float transZ);
    void AddTranslation(const Vec3& InAdditiveTrans);
	const Vec3&	getTranslation() const;
private:
	Vec3 Translation;
	Vec3 RelativeTranslation;
public:
	const Vec3			GetForward() const;
	const Vec3			getUp() const;
	const Vec3			getRight() const;
	const Vec3			getWorldTranslation() const;
	const Mat4&			getWorldMatrix() const;
	virtual Mat4&		getWorldMatrix();
    const Mat4& GetInverseWorldMatrix() const { return InverseWorldMatrix; }
private:
	Mat4 _worldMatrix;
    Mat4 InverseWorldMatrix;

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

    REFLECT(
        SceneComponent,
        PROPERTY(Scale),
        PROPERTY(Rotation),
        PROPERTY(Translation),
        PROPERTY(bUpdated)
    );
};