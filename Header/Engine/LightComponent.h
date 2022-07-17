#pragma once
#ifndef __LIGHT_COMPONENT_H__

#include "PrimitiveComponent.h"

struct PrimitiveData;
class StaticMesh;

class ENGINE_DLL LightComponent : public PrimitiveComponent
{
public:
	explicit LightComponent(void);
	virtual ~LightComponent(void);

public:
	virtual const bool getPrimitiveData(PrimitiveData &primitiveData) override;

public:
	const Vec3&	getColor(void) const;
	void		setColor(const Vec3 &color);
private:
	Vec3		_color;

public:
	void		addIntensity(const float addIntensity);
	void		setIntensity(const float intensity);
	const float	getIntensity();
private:
	float	_intensity;

public:
	void show();
	void hide();
	void toggle();
public:
	const bool isHidden() const;
	const bool isShown() const;
private:
	bool _shown;

protected:
	std::shared_ptr<StaticMesh> getMesh() { return _pStaticMesh; }
private:
	std::shared_ptr<StaticMesh> _pStaticMesh;
};

#define __LIGHT_COMPONENT_H__
#endif