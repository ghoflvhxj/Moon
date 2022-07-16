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

template <class T1, class T2>
inline T1 CastValue(T2 value)
{
	return static_cast<T1>(value);
}

template <class T>
inline int32 enumToInt32(const T enumValue)
{
	return CastValue<int32>(enumValue);
}

template <class  T>
inline uint32 enumToUInt32(const T enumValue)
{
	return CastValue<uint32>(enumValue);
}

template <class T>
inline uint32 enumToIndex(const T enumValue)
{
	return CastValue<uint32>(enumValue);
}

inline float Int32ToFloat(const int32 value)
{
	return CastValue<float>(value);
}

inline bool checkFlag(const uint32 value, const uint32 flag)
{
	return value && flag;
}

/* Com객체 릴리즈에 사용 */
template <class T>
inline void SafeRelease(T &p)
{
	if (nullptr != p)
	{
		ULONG refCount = p->Release();
		if (0 == refCount)
		{
			p = nullptr;
		}
	}
}

template <class T>
inline void SafeReleaseArray(std::vector<T> &arr)
{
	for (T& p : arr)
	{
		SafeRelease(p);
	}

	arr.clear();
}

#define __FUNCTION_H__
#endif
