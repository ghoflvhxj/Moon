#pragma once
#ifndef __BOX_COMPONENT_H__

#include "Vertex.h"

#include "PrimitiveComponent.h"

class TextureComponent;
class Material;

class ENGINE_DLL BoxComponent : public PrimitiveComponent
{
public:
	explicit BoxComponent();
	virtual ~BoxComponent();

public:
	//virtual void Update(const Time deltaTime) override;
	//virtual void render() override;

public:
	void setMaterial(std::shared_ptr<Material> pMaterial);
	std::shared_ptr<Material> &getMaterial();
private:
	std::shared_ptr<Material> _pMaterial;

public:
	void		setColor(const Vec4 &color);
	const Vec4&	getColor(void);
private:
	Vec4 _color;
};

#define __BOX_COMPONENT_H__
#endif