#pragma once

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

enum class ETextureType : uint32
{
	// 기본
	Diffuse
	, Depth
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
		WireFrame,
	    Solid,
	    Count,
	};

	enum class CullMode
	{
		None
		, Frontface
		, Backface
		, Count
	};

	enum class EDepthWriteMode
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