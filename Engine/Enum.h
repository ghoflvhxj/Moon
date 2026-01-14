#pragma once

enum class INPUT_EVENT
{
	PRESSED, RELEASED, END
};

enum class MOUSEBUTTON 
{ 
	LB, RB, MB, EEND 
};

enum class EAxis
{
	X, Y, Z, END
};

enum class EProjectionType
{
    Perspective, Orthograhpic, End
};

enum class ETextureType : uint32
{
	// 기본
	Diffuse
    , Dummy
	, Normal
	, Specular

	// 특수효과
	, Emssive
	, Distortion

	, End
};

enum class ETransform 
{ 
    Scale, 
    Rotation, 
    Translation, 
    End 
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

	enum class EDepthStencilMode
	{
		DepthEnable         = 1 << 1
        , StencilEnable     = 1 << 2
        , StencilReadMask       = 1 << 3
		, DepthDisable      = 1 << 4
        , StencilDisable    = 1 << 5
		, Count             = 1 << 6
	};

	enum class Blend
	{
		Object
		, Light
		, End
	};
}