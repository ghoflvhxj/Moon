#pragma once

#include "Type.h"
#include "Define.h"
#include "Enum.h"

#include <string>
#include <Shlwapi.h>

#include <corecrt_math_defines.h>
using namespace DirectX;

// 점점 기능이 많아지면 클래스의 Static으로 뺴야할 듯

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

template <typename ...Args>
inline uint32 EnumToFlag(Args... args)
{
    uint32 OutFlag = (EnumToIndex(args) | ... | 0);
    return OutFlag;
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

template <class T, size_t N>
inline void SafeReleaseArray(std::array<T, N>& InArray)
{
    for (T& p : InArray)
    {
        SafeRelease(p);
    }
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

inline float ToRadian(float InDegree)
{
    return InDegree * (PI / 180.f);
}

inline Vec3 ToRadian(const Vec3& InVector)
{
    return { ToRadian(InVector.x), ToRadian(InVector.y), ToRadian(InVector.z) };
}

inline float ToDegree(float InRadian)
{
    return InRadian / PI * 180.f;
}

inline Vec3 ToDegree(const Vec3& InVector)
{
    return { ToDegree(InVector.x), ToDegree(InVector.y), ToDegree(InVector.z) };
}

static inline float clampf(float v, float a, float b) 
{ 
    return (v < a) ? a : ((v > b) ? b : v); 
}

inline static std::ostream& operator<<(std::ostream& InOutStream, const Vec3& InVec)
{
    InOutStream << InVec.x << ", " << InVec.y << ", " << InVec.z;
    return InOutStream;
}

inline void DXQuaternionToEuler(::Vec4 q, float& outPitch, float& outYaw, float& outRoll)
{
    // normalize
    float mag = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (mag == 0.0f) { outPitch = outYaw = outRoll = 0.0f; return; }
    float x = q.x / mag, y = q.y / mag, z = q.z / mag, w = q.w / mag;

    // build rotation matrix elements (from quaternion)
    float R00 = 1.0f - 2.0f * (y * y + z * z);
    float R01 = 2.0f * (x * y - w * z);
    float R02 = 2.0f * (x * z + w * y);

    float R10 = 2.0f * (x * y + w * z);
    float R11 = 1.0f - 2.0f * (x * x + z * z);
    float R12 = 2.0f * (y * z - w * x);

    float R20 = 2.0f * (x * z - w * y);
    float R21 = 2.0f * (y * z + w * x);
    float R22 = 1.0f - 2.0f * (x * x + y * y);

    // pitch = asin(R21)
    float sin_pitch = clampf(R21, -1.0f, 1.0f);
    outPitch = asinf(sin_pitch);

    const float SINGULAR_EPS = 1.0f - 1e-6f;
    if (sin_pitch > SINGULAR_EPS) {
        // +90 deg singularity
        outPitch = (float)M_PI_2; // +pi/2
        // we lose one DOF; set yaw from one stable formula, roll = 0 (or some convention)
        outYaw = atan2f(-R20, R22);
        outRoll = 0.0f;
    }
    else if (sin_pitch < -SINGULAR_EPS) {
        // -90 deg singularity
        outPitch = (float)-M_PI_2;
        outYaw = atan2f(-R20, R22);
        outRoll = 0.0f;
    }
    else {
        // general case
        outYaw = atan2f(-R20, R22);
        outRoll = atan2f(-R01, R11);
    }
}

inline Vec4 EulerToQuaternion(const Vec3& InEulerAngles)
{
    Vec4 OutQuat = {};
    XMStoreFloat4(&OutQuat, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&InEulerAngles)));

    return OutQuat;
}

inline Vec3 GetAxis(const Mat4& InMatrix, const EAxis InAxis)
{
    uint32 Index = EnumToIndex(InAxis);
    return { InMatrix.m[Index][0], InMatrix.m[Index][1], InMatrix.m[Index][2] };
}

inline Vec3 GetPos(const Mat4& InMatrix)
{
    return { InMatrix.m[3][0], InMatrix.m[3][1], InMatrix.m[3][2] };
}

inline void DecomposeTransform(const Mat4& InMatrix, Vec3& OutScale, Vec3& OutRot, Vec3& OutTrans)
{
    XMVECTOR XMScale, XMRot, XMTrans;
    XMVECTOR XMRotQuat;
    XMMatrixDecompose(&XMScale, &XMRotQuat, &XMTrans, XMLoadFloat4x4(&InMatrix));

    XMTrans /= XMScale;

    Vec4 RotQuat = {};
    XMStoreFloat4(&RotQuat, XMRotQuat);

    Vec3 Rot = {};
    DXQuaternionToEuler(RotQuat, Rot.x, Rot.y, Rot.z);
    XMRot = XMLoadFloat3(&Rot);

    XMStoreFloat3(&OutScale, XMScale);
    XMStoreFloat3(&OutRot, XMRot);
    XMStoreFloat3(&OutTrans, XMTrans);
}

inline void TransformMatrix(Mat4& OutMatrix, const Vec3& InScale, const Vec3& InRotation, const Vec3& InTranslation)
{
    const Vec4& QuatRot = EulerToQuaternion(InRotation);

    XMVECTOR vectors[(int)ETransform::End] = {
        XMLoadFloat3(&InScale),
        XMLoadFloat4(&QuatRot),
        XMLoadFloat3(&InTranslation)
    };

    XMStoreFloat4x4(&OutMatrix, XMMatrixScalingFromVector(vectors[(int)ETransform::Scale]) * XMMatrixRotationQuaternion(vectors[(int)ETransform::Rotation]) * XMMatrixTranslationFromVector(vectors[(int)ETransform::Translation]));
}

inline void TransformMatrix(Mat4& OutMatrix, const Vec3& InScale, const Vec4& InQuatRotation, const Vec3& InTranslation)
{
    XMVECTOR vectors[(int)ETransform::End] = {
        XMLoadFloat3(&InScale),
        XMLoadFloat4(&InQuatRotation),
        XMLoadFloat3(&InTranslation)
    };

    XMStoreFloat4x4(&OutMatrix, XMMatrixScalingFromVector(vectors[(int)ETransform::Scale]) * XMMatrixRotationQuaternion(vectors[(int)ETransform::Rotation]) * XMMatrixTranslationFromVector(vectors[(int)ETransform::Translation]));
}