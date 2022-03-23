#pragma once
#ifndef __POINTLIGHT_COMPONENT_H__

#include "Vertex.h"

#include "LightComponent.h"

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

class TextureComponent;

class ENGINE_DLL PointLightComponent : public LightComponent
{
public:
	struct VertexShaderConstantBuffer
	{
		Mat4 worldMatrix;
		Mat4 viewMatrix;
		Mat4 projectionMatrix;
	};

	__declspec(align(16)) struct PixelShaderConstantBuffer
	{
		Vec4	g_lightPosition;	// w = Range
		Vec4	g_lightColor;		// w = Power

		Mat4 g_inverseCameraViewMatrix;
		Mat4 g_inverseProjectiveMatrix;
	};

public:
	explicit PointLightComponent(void);
	virtual ~PointLightComponent(void);

private:
	void initializeVertices();
private:
	std::vector<Vertex> _vertexList;
	std::vector<Index>	_indexList;

private:
	void initializeBuffers();
private:
	std::shared_ptr<VertexBuffer> _pVertexBuffer;
	std::shared_ptr<IndexBuffer> _pIndexBuffer;
	std::shared_ptr<ConstantBuffer> _pVertexConstantBuffer;
	std::shared_ptr<ConstantBuffer> _pPixelConstantBuffer;

private:
	std::shared_ptr<TextureComponent> _pTextureComponent;

public:
	virtual void render() override;

public:
	void		addRange(const float addRange);
	void		setRange(const float range);
	const float	getRange() const;
private:
	float	_range;

private:

};

#define __POINTLIGHT_COMPONENT_H__
#endif