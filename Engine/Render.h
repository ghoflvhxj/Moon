#pragma once
#ifndef __RENDER_H__

#include "GraphicDevice.h"
#include "ShaderManager.h"

#define RENDERER_OPTION(name)	bool b##name = false;\
								void Set##name(bool New##name) { b##name = New##name; }\
								bool Is##name() { return b##name; }

// 프레임워크
class PrimitiveComponent;
class MTexture;

// 렌더
class RenderTarget;
class RenderPass;

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

	DirectionalShadowDepth,
	PointShadowDepth,

    Collision,

	Count
};

enum class ERenderPass
{
	ShadowDepth,
	PointShadowDepth,
	Geometry,
	Light,
	SkyPass,
    Collision,
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
	DirectionalLight,
    PointLight,
	
	Collision,

	Count
};

struct FMeshData;
class PrimitiveComponent;
class MIndexBuffer;
class MVertexBuffer;
class MMaterial;
class MShader;

struct FPrimitiveData
{
    FPrimitiveData()
        : _primitiveType(EPrimitiveType::Count), _jointCount(0), _matrices(nullptr) 
    {}

	std::weak_ptr<PrimitiveComponent>			_pPrimitive;
	std::weak_ptr<MMaterial>					_pMaterial;
	EPrimitiveType _primitiveType;

	// 메시가 채우는 데이터
	std::weak_ptr<FMeshData>					MeshData;

	// 렌더러가 채워줘야 하는 데이터
	std::shared_ptr<MVertexBuffer>				VertexBuffer;
	std::shared_ptr<MIndexBuffer>				IndexBuffer;

	// 다이나믹 메쉬용
	Mat4* _matrices = nullptr;
	uint32 _jointCount;
};

#define __RENDER_H__
#endif