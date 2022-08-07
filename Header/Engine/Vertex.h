#pragma once
#ifndef __VERTEX_H__

#include <DirectXMath.h>

namespace Graphic
{
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
		Vec3 Pos = { 0.f, 0.f, 0.f };
		Vec4 Color = { 1.f, 1.f, 1.f, 1.f };
		Vec2 Tex0 = { 0.f, 0.f };
		Vec3 Normal = { 0.f, 0.f, 0.f };
		Vec3 Tangent = { 0.f, 0.f, 0.f };
		Vec3 Binormal = { 0.f, 0.f, 0.f };
		uint32 BlendIndex[4] = { 0, 0, 0, 0 };
		Vec4 BlendWeight = { 0.f, 0.f, 0.f, 0.f };

		static void getDesc(std::vector<D3D11_INPUT_ELEMENT_DESC> &inputDescVector)
		{
			D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 88, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			size_t elementCount = sizeof(inputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

			inputDescVector.reserve(elementCount);
			inputDescVector.assign(std::begin(inputDesc), std::end(inputDesc));
		}
	};
}

using Vertex	= Graphic::VERTEX_COMMON;
using Index		= uint32;

using VertexList	= std::vector<Vertex>;
using IndexList		= std::vector<Index>;

class TextureComponent;
using TextureList	= std::vector<std::shared_ptr<TextureComponent>>;

class Material;
using MaterialList	= std::vector<std::shared_ptr<Material>>;

#define __VERTEX_H__
#endif