#pragma once

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
    Stencil,
    Outline,
	Count
};

enum class ERenderPass
{
	ShadowDepth,
	PointShadowDepth,
	Geometry,
    Stencil,
	DirectionalLight,
    PointLight,
	SkyPass,
    Collision,
    Outline,
	Combine,
    EditorGizmo,
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
    {
    }

    template <class T>
    std::shared_ptr<T> GetPrimitiveComponent() const
    {
        return PrimitiveComponent.lock()->template CastTo<T>();
    }

	std::weak_ptr<MPrimitiveComponent>			PrimitiveComponent;
	std::weak_ptr<MMaterial>					Material;
	EPrimitiveType PrimitiveType = EPrimitiveType::Count;

	// 메시가 채우는 데이터
	const FMeshData* MeshData = nullptr;

	// 렌더러가 채워줘야 하는 데이터
	std::weak_ptr<MVertexBuffer>			VertexBuffer;
	std::weak_ptr<MIndexBuffer>				IndexBuffer;

	// 다이나믹 메쉬용
	Mat4* AnimMatrices = nullptr;
	uint32 _jointCount = 0;

    struct ID3D11InputLayout* InputLayout = nullptr;
};
