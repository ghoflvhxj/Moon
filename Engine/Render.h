#pragma once
#ifndef __RENDER_H__

#include "GraphicDevice.h"
#include "ShaderManager.h"

// 프레임워크
class PrimitiveComponent;
class TextureComponent;

// 렌더
class RenderTarget;
class RenderPass;

using RenderQueue	= std::vector<std::shared_ptr<PrimitiveComponent>>;
using RenderTargets = std::vector<std::shared_ptr<RenderTarget>>;

enum class ERenderTarget
{
	//DepthPre,
	Diffuse,
	Depth,
	Normal,
	Specular,

	LightDiffuse,
	LightSpecular,

	Count
};

enum class ERenderPass
{
	Geometry,
	Light,
	SkyPass,
	Combine,
	Count
};

enum class CBufferElementType
{
	Int,
	Float,
	Double,
	Count
};

enum class EPrimitiveType
{
	// Mesh
	Mesh,
	Sky,

	// Light
	Light,
	
	Count
};

class PrimitiveComponent;
class IndexBuffer;
class VertexBuffer;
class Material;
class Shader;

struct PrimitiveData
{
	std::shared_ptr<PrimitiveComponent>			_pPrimitive;
	std::shared_ptr<VertexBuffer>				_pVertexBuffer;
	std::shared_ptr<IndexBuffer>				_pIndexBuffer;
	std::shared_ptr<Material>					_pMaterial;
	EPrimitiveType _primitiveType;

	// 쉐이더를 강제로 할당하고 싶을 때
	std::shared_ptr<Shader>						_pVertexShader = nullptr;
	std::shared_ptr<Shader>						_pPixelShader = nullptr;
	
	// 다이나믹 메쉬용
	Mat4 *_matrices = nullptr;
	uint32 _jointCount;
};

#define __RENDER_H__
#endif