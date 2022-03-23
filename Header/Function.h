#pragma once
#ifndef __FUNCTION_H__

inline void GetResourceDirectory(WCHAR buffer[])
{
	GetCurrentDirectory(MAX_PATH, buffer);
	PathCombine(buffer, buffer, RESOURCE_DIRECTORY);
}

inline void getShaderDirectory(WCHAR buffer[])
{
	GetResourceDirectory(buffer);
	PathCombine(buffer, buffer, SHADER_DIRECTORY);
}

inline void getVertexShaderDirectory(WCHAR buffer[])
{
	getShaderDirectory(buffer);
	PathCombine(buffer, buffer, SHADER_VERTEX_DIRECTORY);
}

inline void getPixelShaderDirectory(WCHAR buffer[])
{
	getShaderDirectory(buffer);
	PathCombine(buffer, buffer, SHADER_PIXEL_DIRECTORY);
}

template <class T>
inline int32 enumToInt32(const T enumValue)
{
	return static_cast<int32>(enumValue);
}

template <class  T>
inline uint32 enumToUInt32(const T enumValue)
{
	return static_cast<uint32>(enumValue);
}

template <class T>
inline uint32 enumToIndex(const T enumValue)
{
	return enumToUInt32(enumValue);
}

template <class T>
inline uint32 enumSize()
{
	return enumToUInt32(T::End);
}

inline float Int32ToFloat(const int32 value)
{
	return static_cast<float>(value);
}

template <class T>
inline T int32ToEnum(const int32 value)
{
	return static_cast<T>(value);
}

template <class T>
inline T uint32ToEnum(const uint32 value)
{
	return static_cast<T>(value);
}

inline uint32 sizeToUint32(const size_t value)
{
	return static_cast<uint32>(value);
}

inline bool checkFlag(const uint32 value, const uint32 flag)
{
	return value && flag;
}

/* Com객체 릴리즈에 사용 */
template <class T>
inline void SafeRelease(T * &p)
{
	if (nullptr != p)
	{
		p->Release();
		p = nullptr;
	}
}

#define __FUNCTION_H__
#endif
