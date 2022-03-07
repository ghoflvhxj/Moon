#pragma once
#ifndef __VERTEX_H__

#include <DirectXMath.h>

namespace Grahpic
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

	struct VERTEX
	{
		Vec3 Pos;
		Vec4 Color;
		Vec2 Tex0;
		Vec3 Normal;
		Vec3 Tangent;
		Vec3 Binormal;

		static void getDesc(std::vector<D3D11_INPUT_ELEMENT_DESC> &inputDescVector)
		{
			D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			size_t elementCount = sizeof(inputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

			inputDescVector.reserve(elementCount);
			inputDescVector.assign(std::begin(inputDesc), std::end(inputDesc));
		}
	};

}

using Vertex	= Grahpic::VERTEX;
using Index		= uint32;

using VertexList	= std::vector<Vertex>;
using IndexList		= std::vector<Index>;

class TextureComponent;
using TextureList	= std::vector<std::shared_ptr<TextureComponent>>;

class Material;
using MaterialList	= std::vector<std::shared_ptr<Material>>;

#define __VERTEX_H__
#endif