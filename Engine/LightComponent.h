#pragma once

#include "PrimitiveComponent.h"

struct FPrimitiveData;
class StaticMesh;

class ENGINE_DLL MLightComponent : public MPrimitiveComponent
{
public:
	explicit MLightComponent(void);
	virtual ~MLightComponent(void);

public:
    virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;

public:
    virtual Mat4& getWorldMatrix() override;
protected:
    Mat4 LightWorldMatrix;

public:
    const Vec3& GetDirection() const { return Direction; }
protected:
    Vec3 Direction = { 0.f, -1.f, 0.f};

public:
	const Vec3&	getColor(void) const;
	void		setColor(const Vec3 &color);
private:
	Vec3 Color;

public:
	void		addIntensity(const float addIntensity);
	void		setIntensity(const float intensity);
	const float	getIntensity();
private:
	float Intensity;

public:
	void show();
	void hide();
	void toggle();
public:
	const bool isHidden() const;
	const bool isShown() const;
private:
	bool bShow;

protected:
	std::shared_ptr<StaticMesh> getMesh() { return _pStaticMesh; }
private:
	std::shared_ptr<StaticMesh> _pStaticMesh;

protected:
    std::shared_ptr<MMaterial> Material;

    REFLECT(
        MLightComponent,
        PROPERTY(Color),
        PROPERTY(Direction),
        PROPERTY(Intensity),
        PROPERTY(bShow)
    )
};
