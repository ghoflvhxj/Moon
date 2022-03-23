#pragma once
#ifndef __DIRECTIONAL_LIGHT_COMPONENT_H__

#include "LightComponent.h"

#include "Vertex.h"

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

class TextureComponent;

class ENGINE_DLL DirectionalLightComponent : public LightComponent
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
		Vec4	g_lightDirection;	// w = Range
		Vec4	g_lightColor;		// w = Power

		Mat4 g_inverseCameraViewMatrix;
		Mat4 g_inverseProjectiveMatrix;
	};

public:
	explicit DirectionalLightComponent(void);
	virtual ~DirectionalLightComponent(void);

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

public:
	virtual void render() override;
};

#define __DIRECTIONAL_LIGHT_COMPONENT_H__
#endif