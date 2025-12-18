#pragma once

#include "Include.h"

constexpr UINT DefaultSlot = 0;
constexpr UINT InstanceSlot = 1;

namespace Graphic
{
    struct VERTEX_SIMPLE
    {
        Vec4 Pos;
    };

	struct VERTEX_COLOR
	{
		Vec3 Pos;
		Vec4 Color;
	};

	struct VERTEX_COLOR_TEX
	{
		Vec3 Pos;
		Vec4 Color;
		Vec2 Tex0;
	};

	struct VERTEX_COLOR_TEX_NORMAL
	{
		Vec3 Pos;
		Vec4 Color;
		Vec2 Tex0;
		Vec3 Normal;
	};

	__declspec(align(16)) struct VERTEX_COMMON
	{
        VERTEX_COMMON() = default;
        VERTEX_COMMON(const Vec4& pos)
            : Pos(pos)
        {
        }
        VERTEX_COMMON(const VERTEX_COMMON& Rhs) = default;

		Vec4 Pos = { 0.f, 0.f, 0.f, 1.f };
		Vec4 Color = { 1.f, 1.f, 1.f, 1.f };
		Vec2 Tex0 = { 0.f, 0.f };   
		Vec3 Normal = { 0.f, 0.f, 0.f };
		Vec3 Tangent = { 0.f, 0.f, 0.f };
		Vec3 Binormal = { 0.f, 0.f, 0.f };
		int32 BlendIndex[4] = { -1, -1, -1, -1 };
		float BlendWeight[4] = {0.f, 0.f, 0.f, 0.f};

        // 가상함수 등이 추가되면 vtable때문에 사이즈가 달라지므로, 리플렉션에 직접 등록할 수는 없음.
        //REFLECT_TOP(
        //    VERTEX_COMMON, 
        //);
	};
}

inline void getDesc(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDescVector)
{
    D3D11_INPUT_ELEMENT_DESC InputDescs[] = {
        // 버텍스
        {"POSITION",        0, DXGI_FORMAT_R32G32B32A32_FLOAT,  DefaultSlot, 0,     D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",           0, DXGI_FORMAT_R32G32B32A32_FLOAT,  DefaultSlot, 16,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",        0, DXGI_FORMAT_R32G32_FLOAT,        DefaultSlot, 32,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",          0, DXGI_FORMAT_R32G32B32_FLOAT,     DefaultSlot, 40,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",          1, DXGI_FORMAT_R32G32B32_FLOAT,     DefaultSlot, 52,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",          2, DXGI_FORMAT_R32G32B32_FLOAT,     DefaultSlot, 64,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDINDICES",    0, DXGI_FORMAT_R32G32B32A32_SINT,   DefaultSlot, 76,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  DefaultSlot, 92,    D3D11_INPUT_PER_VERTEX_DATA, 0},

        // 인스턴스
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, InstanceSlot, 0,     D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, InstanceSlot, 16,    D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, InstanceSlot, 32,    D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, InstanceSlot, 48,    D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    size_t elementCount = sizeof(InputDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    inputDescVector.reserve(elementCount);
    inputDescVector.assign(std::begin(InputDescs), std::end(InputDescs));
}

__declspec(align(16)) struct FVertex_Instance
{
    Mat4 WorldMatrix = IDENTITYMATRIX;
};

// VERTEX_COMMON 리플렉션
struct FTypeDesc;
template <>
const FTypeDesc* GetTypeDesc<Graphic::VERTEX_COMMON>();

using Vertex	= Graphic::VERTEX_COMMON;
using Index		= uint32;

using VertexList	= std::vector<Vertex>;
using IndexList		= std::vector<Index>;

class MTexture;
// TextureType을 인덱스로 텍스쳐를 저장
using TextureList	= std::vector<std::shared_ptr<MTexture>>;

class MMaterial;
using MaterialList	= std::vector<std::shared_ptr<MMaterial>>;
