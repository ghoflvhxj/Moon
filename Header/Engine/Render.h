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

class IndexBuffer;
class VertexBuffer;
class Material;
class Shader;

struct PrimitiveData
{
	std::vector<std::shared_ptr<VertexBuffer>>	_pVertexBuffers;
	std::shared_ptr<IndexBuffer>				_pIndexBuffer;
	std::vector<std::shared_ptr<Material>>		_pMaterials;
	std::shared_ptr<Shader>						_pVertexShader;
	std::shared_ptr<Shader>						_pPixelShader;
	std::vector<uint32>							_geometryMaterialLinkIndex;
	EPrimitiveType _primitiveType;
};

#define __RENDER_H__
#endif