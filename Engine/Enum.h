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
	// �⺻
	Diffuse
	, Normal
	, Specular

	// Ư��ȿ��
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
		, End
	};

	enum class CullMode
	{
		None
		, Frontface
		, Backface
		, End
	};

	enum class DepthWriteMode
	{
		Enable
		, Disable
		, End
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