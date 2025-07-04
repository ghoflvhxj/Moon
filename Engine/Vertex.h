#pragma once

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

        REFLECTABLE(
            REFLECT_FIELD(VERTEX_COMMON, Pos),
            REFLECT_FIELD(VERTEX_COMMON, Tex0),
            REFLECT_FIELD(VERTEX_COMMON, Normal),
            REFLECT_FIELD(VERTEX_COMMON, Tangent),
            REFLECT_FIELD(VERTEX_COMMON, Binormal),
            REFLECT_FIELD(VERTEX_COMMON, BlendIndex),
            REFLECT_FIELD(VERTEX_COMMON, BlendWeight)
        );
	};
}

using Vertex	= Graphic::VERTEX_COMMON;
using Index		= uint32;

using VertexList	= std::vector<Vertex>;
using IndexList		= std::vector<Index>;

class MTexture;
// TextureType을 인덱스로 텍스쳐를 저장
using TextureList	= std::vector<std::shared_ptr<MTexture>>;

class MMaterial;
using MaterialList	= std::vector<std::shared_ptr<MMaterial>>;
