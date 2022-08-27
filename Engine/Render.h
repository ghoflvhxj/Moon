#pragma once
#ifndef __RENDER_H__

#include "GraphicDevice.h"
#include "ShaderManager.h"

// �����ӿ�ũ
class PrimitiveComponent;
class TextureComponent;

// ����
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

	// ���̴��� ������ �Ҵ��ϰ� ���� ��
	std::shared_ptr<Shader>						_pVertexShader = nullptr;
	std::shared_ptr<Shader>						_pPixelShader = nullptr;
	
	// ���̳��� �޽���
	Mat4 *_matrices = nullptr;
	uint32 _jointCount;
};

#define __RENDER_H__
#endif