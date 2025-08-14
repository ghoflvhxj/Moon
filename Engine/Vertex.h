#pragma once

#include "Include.h"

namespace Graphic
{
    struct VERTEX_SIMPLE
    {
        Vec4 Pos;
        static void getDesc(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDescVector)
        {
            D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
            };

            inputDescVector.assign(std::begin(inputDesc), std::end(inputDesc));
        }

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
        VERTEX_COMMON(const VERTEX_COMMON& Rhs)
            : Pos(Rhs.Pos), Color(Rhs.Color), Tex0(Rhs.Tex0),
              Normal(Rhs.Normal), Tangent(Rhs.Tangent), Binormal(Rhs.Binormal),
              BlendIndex{ Rhs.BlendIndex[0], Rhs.BlendIndex[1], Rhs.BlendIndex[2], Rhs.BlendIndex[3] },
              BlendWeight{ Rhs.BlendWeight[0], Rhs.BlendWeight[1], Rhs.BlendWeight[2], Rhs.BlendWeight[3] }
        {
        }

		Vec4 Pos = { 0.f, 0.f, 0.f, 1.f };
		Vec4 Color = { 1.f, 1.f, 1.f, 1.f };
		Vec2 Tex0 = { 0.f, 0.f };
		Vec3 Normal = { 0.f, 0.f, 0.f };
		Vec3 Tangent = { 0.f, 0.f, 0.f };
		Vec3 Binormal = { 0.f, 0.f, 0.f };
		uint32 BlendIndex[4] = { 0, 0, 0, 0 };
		float BlendWeight[4] = {0.f, 0.f, 0.f, 0.f};

		static void getDesc(std::vector<D3D11_INPUT_ELEMENT_DESC> &inputDescVector)
		{
			D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 76, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 92, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			size_t elementCount = sizeof(inputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

			inputDescVector.reserve(elementCount);
			inputDescVector.assign(std::begin(inputDesc), std::end(inputDesc));
		}

        // 가상함수 등이 추가되면 vtable때문에 사이즈가 달라짐...
        //REFLECT_TOP(
        //    VERTEX_COMMON, 
        //);
	};
}

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
