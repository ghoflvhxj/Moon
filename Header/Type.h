#pragma once
#ifndef __TYPE_H__

#include <chrono>
#include <DirectXMath.h>

using Byte		= unsigned char;
using uint8		= unsigned char;
using uint32	= unsigned int;
using int32		= int;

using Frame		= unsigned int;
using Time		= float;
using TimeClock = std::chrono::system_clock::time_point;
using TimerHandle = unsigned int;

using DikState = Byte;
using Dik = Byte;

using Vec4 = DirectX::XMFLOAT4;
using Vec3 = DirectX::XMFLOAT3;
using Vec2 = DirectX::XMFLOAT2;

using Mat4 = DirectX::XMFLOAT4X4;

using DIRECTORY = const wchar_t *;


#endif __TYPE_H__