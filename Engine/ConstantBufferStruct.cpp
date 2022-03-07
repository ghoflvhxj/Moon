#include "stdafx.h"
#include "ConstantBufferStruct.h"

using namespace ConstantBufferStruct::VertexStruct;
using namespace ConstantBufferStruct::PixelStruct;

ViewProjectionMatrix::ViewProjectionMatrix(const Mat4 &worldViewMatrix, const Mat4 &projectionMatrix)
	: _worldViewMatrix(worldViewMatrix)
	, _projectionMatrix(projectionMatrix)
{

}

void ViewProjectionMatrix::update()
{

}

//bool TextureFlag::checkTextureFlag(TextureFlag flag)
//{
//	return checkFlag(_textureFlag, enumToUInt32(flag));
//}

WolrdMatrix::WolrdMatrix(const Mat4 &worldMatrix)
	: _worldMatrix(worldMatrix)
{

}

void WolrdMatrix::update()
{

}
