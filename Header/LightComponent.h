#pragma once
#ifndef __LIGHT_COMPONENT_H__

#include "SceneComponent.h"

class ENGINE_DLL LightComponent : public SceneComponent, public std::enable_shared_from_this<LightComponent>
{
public:
	explicit LightComponent(void);
	virtual ~LightComponent(void);

public:
	virtual void Update(const Time deltaTime) override;
public:
	virtual void render() = 0;

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
};

#define __LIGHT_COMPONENT_H__
#endif