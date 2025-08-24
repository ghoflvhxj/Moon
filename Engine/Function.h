#pragma once
#include "Define.h"
#include <string>

#include <Shlwapi.h>

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

inline void getGeometryShaderDirectory(WCHAR buffer[])
{
	getShaderDirectory(buffer);
	PathCombine(buffer, buffer, SHADER_GEOMETRY_DIRECTORY);
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
inline uint32 EnumToIndex(const T enumValue)
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

template <class T>
inline uint32 GetSize(const T& Container)
{
    return static_cast<uint32>(Container.size());
}

/* Com객체 릴리즈에 사용 */
template <class T>
inline void SafeRelease(T &p)
{
	if (nullptr != p)
	{
		ULONG refCount = p->Release();
#ifdef DEBUG
		//std::wstring DebugMessage = TEXT("SafeRelease : ") + std::to_wstring(refCount) + TEXT("\n");
		//OutputDebugStringW(DebugMessage.c_str());
#endif
		p = nullptr;
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

inline void WStringToString(const std::wstring& wstr, char Buffer[], size_t BufferSize)
{
	memset(Buffer, 0, BufferSize);
	int Length = static_cast<int>(wstr.length()) + 1;
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), Length, Buffer, Length * 2, nullptr, nullptr);
}

inline std::string WStringToString(const std::wstring& InString)
{
    std::string Temp;
    Temp.resize(InString.length());
    WStringToString(InString, Temp.data(), Temp.length());
    return Temp;
}

inline void StringToWString(const char* Buffer, std::wstring& wstr)
{
    /*
	int BufferSize = static_cast<int>(strlen(Buffer));
	wstr.resize(BufferSize);
	MultiByteToWideChar(CP_ACP, 0, Buffer, BufferSize, wstr.data(), BufferSize * 2);
    */
    if (wstr.empty() == false)
    {
        wstr.clear();
    }

    std::string Temp = Buffer;
    if (Temp.empty())
    {
        return;
    }

    wstr.assign(Temp.begin(), Temp.end());
}

inline void StringToWString(const std::string& Buffer, std::wstring& wstr)
{
    StringToWString(Buffer.c_str(), wstr);
}

inline std::wstring StringToWString(const char* Buffer)
{
    std::wstring NewString;
    int BufferSize = static_cast<int>(strlen(Buffer));
    NewString.resize(BufferSize);
    MultiByteToWideChar(CP_ACP, 0, Buffer, BufferSize, NewString.data(), BufferSize * 2);

    return NewString;
}

inline std::wstring StringToWString(const std::string& Buffer)
{
    return StringToWString(Buffer.c_str());
}