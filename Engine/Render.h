#pragma once
#ifndef __RENDER_H__

#include "GraphicDevice.h"
#include "ShaderManager.h"

#define RENDERER_OPTION(name)	bool b##name = false;\
								void Set##name(bool New##name) { b##name = New##name; }\
								bool Is##name() { return b##name; }

// 프레임워크
class MPrimitiveComponent;
class MTexture;

// 렌더
class RenderTarget;
class MRenderPass;

using RenderTargets = std::vector<std::shared_ptr<RenderTarget>>;

// PixelShader랑 맞춰줘야 함
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

    PointLightDiffuse,

	Count
};

enum class ERenderPass
{
	ShadowDepth,
	PointShadowDepth,
	Geometry,
	DirectionalLight,
    PointLight,
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

//std::vector<EPrimitiveType> EPrimitiveTypes = { EPrimitiveType::Mesh, EPrimitiveType::Sky, EPrimitiveType::DirectionalLight, EPrimitiveType::PointLight, EPrimitiveType::Collision };

struct FMeshData;
class MPrimitiveComponent;
class MIndexBuffer;
class MVertexBuffer;
class MMaterial;
class MShader;

struct FPrimitiveData
{
    FPrimitiveData()
        : PrimitiveType(EPrimitiveType::Count), _jointCount(0), _matrices(nullptr) 
    {}

    template <class T>
    const std::shared_ptr<T> GetPrimitiveComponent() const
    {
        return std::static_pointer_cast<T>(PrimitiveComponent.lock());
    }

	std::weak_ptr<MPrimitiveComponent>			PrimitiveComponent;
	std::weak_ptr<MMaterial>					Material;
	EPrimitiveType PrimitiveType;

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