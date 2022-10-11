#pragma once
#ifndef __DEFINE_H__

constexpr wchar_t DEFAULT_CLASSNAME[]	= TEXT("DefaultClass");
constexpr wchar_t ROOT_COMPONENT[]		= TEXT("RootComponent");

constexpr wchar_t EMPTY_TEXT_W[]		= TEXT("");
constexpr char EMPTY_TEXT_A[]			= "";

constexpr DIRECTORY RESOURCE_DIRECTORY			= TEXT("Resources");
constexpr DIRECTORY SHADER_DIRECTORY			= TEXT("Shader");
constexpr DIRECTORY SHADER_VERTEX_DIRECTORY		= TEXT("Vertex");
constexpr DIRECTORY SHADER_PIXEL_DIRECTORY		= TEXT("Pixel");
constexpr DIRECTORY SHADER_GEOMETRY_DIRECTORY	= TEXT("Geometry");

constexpr float FLOAT3_ZERO[] = { 0.f, 0.f, 0.f };
constexpr float FLOAT3_ONE[] = { 1.f, 1.f, 1.f };
constexpr float FLOAT3_UP[] = { 0.f, 1.f, 0.f };

constexpr Vec3 VEC3UP	= { 0.f, 1.f, 0.f };
constexpr Vec3 VEC3ZERO = { 0.f, 0.f, 0.f };
constexpr Vec4 VEC4ZERO = { 0.f, 0.f, 0.f, 0.f };

constexpr Vec4 IDENTITY0 = { 1.f, 0.f, 0.f, 0.f };
constexpr Vec4 IDENTITY1 = { 0.f, 1.f, 0.f, 0.f };
constexpr Vec4 IDENTITY2 = { 0.f, 0.f, 1.f, 0.f };
constexpr Vec4 IDENTITY3 = { 0.f, 0.f, 0.f, 1.f };

constexpr Mat4 IDENTITYMATRIX = {	1.f, 0.f, 0.f, 0.f,
									0.f, 1.f, 0.f, 0.f,
									0.f, 0.f, 1.f, 0.f,
									0.f, 0.f, 0.f, 1.f };

constexpr Mat4 ZEROMATRIX = {	0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f,
								0.f, 0.f, 0.f, 0.f };

constexpr uint32 TEXTURE_COUNT = static_cast<uint32>(TextureType::End);

namespace EngineColors
{
	XMGLOBALCONST DirectX::XMVECTORF32 White		= { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Black		= { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Red		= { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Green		= { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Blue		= { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Yellow	= { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Cyan		= { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Magenta	= { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST DirectX::XMVECTORF32 Silver	= { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}

#define __DEFINE_H__
#endif