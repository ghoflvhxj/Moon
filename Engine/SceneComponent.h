#pragma once

#include "Component.h"

class ENGINE_DLL MSceneComponent : public MComponent
{
public:
	explicit MSceneComponent();
	virtual ~MSceneComponent();

public:
	virtual void Update(const Time deltaTime);
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
	virtual void SetRotation(const Vec3& InRotation);
    virtual void AddRotation(const Vec3& InAdditiveRot);
	const Vec3&	getRotation() const;
    const Vec3 GetWorldRotation() const;
protected:
	Vec3 Rotation;
    bool bWorldRotation = false;

public:
	virtual void setTranslation(const Vec3 &translation);
	void setTranslation(const float transX, const float transY, const float transZ);
    void AddTranslation(const Vec3& InAdditiveTrans);
	const Vec3&	getTranslation() const;
private:
	Vec3 Translation;

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
	void AddChildComponent(std::shared_ptr<MSceneComponent> Component);
    const std::vector<std::weak_ptr<MSceneComponent>>& GetChildComponents() const { return ChildComponents; }
private:
	std::vector<std::weak_ptr<MSceneComponent>> ChildComponents;
    std::weak_ptr<MSceneComponent> ParentComponent;

    REFLECT(
        MSceneComponent,
        PROPERTY(Scale),
        PROPERTY(Rotation),
        PROPERTY(bWorldRotation),
        PROPERTY(Translation),
        PROPERTY(bUpdated)
    );
};