#pragma once
#ifndef __ENUM_H__

enum class INPUT_EVENT
{
	PRESSED, RELEASED, END
};

enum class MOUSEBUTTON 
{ 
	LB, RB, MB, EEND 
};

enum class MOUSEAXIS
{
	X, Y, Z, END
};

enum class TextureType : uint32
{
	// 기본
	Diffuse
	, Normal
	, Specular

	// 특수효과
	, Distortion
	, Emssive

	, End
};

namespace Graphic
{
	enum class FillMode
	{
		WireFrame
		, Solid
		, Count
	};

	enum class CullMode
	{
		None
		, Frontface
		, Backface
		, Count
	};

	enum class DepthWriteMode
	{
		Enable
		, Disable
		, Count
	};

	enum class Blend
	{
		Object
		, Light
		, End
	};
}

#define __ENUM_H__
#endif