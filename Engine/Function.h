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

// YXZ 쿼터니언으로부터 YXZ 앵글을 얻음
inline void DXQuaternionToEuler(::Vec4 q, float& outPitch, float& outYaw, float& outRoll)
{
    // Normalize to be safe
    XMVECTOR qn = XMQuaternionNormalize(XMLoadFloat4(&q));

    float qx = XMVectorGetX(qn);
    float qy = XMVectorGetY(qn);
    float qz = XMVectorGetZ(qn);
    float qw = XMVectorGetW(qn);

    // Rotation matrix elements (row-major) from quaternion (Wikipedia form)
    float r00 = 1.0f - 2.0f * (qy * qy + qz * qz);
    float r01 = 2.0f * (qx * qy - qw * qz);
    float r02 = 2.0f * (qw * qy + qx * qz);

    float r10 = 2.0f * (qx * qy + qw * qz);
    float r11 = 1.0f - 2.0f * (qx * qx + qz * qz);
    float r12 = 2.0f * (qy * qz - qw * qx);

    float r20 = 2.0f * (qx * qz - qw * qy);
    float r21 = 2.0f * (qw * qx + qy * qz);
    float r22 = 1.0f - 2.0f * (qx * qx + qy * qy);

    // For Y * X * Z extraction (yaw = Y, pitch = X, roll = Z):
    // pitch (x) = asin( -r12 )
    // roll  (z) = atan2( r10, r11 )
    // yaw   (y) = atan2( r02, r22 )
    //
    // handle numerical clamping and gimbal-lock when cos(pitch) ~= 0
    const float EPS = 1e-6f;

    // clamp input to asin to [-1,1]
    float sinx = -r12;
    if (sinx > 1.0f) sinx = 1.0f;
    if (sinx < -1.0f) sinx = -1.0f;

    float pitch = asinf(sinx);
    float cosx = cosf(pitch);

    float yaw, roll;
    if (fabsf(cosx) > EPS) {
        roll = atan2f(r10, r11);      // Z
        yaw = atan2f(r02, r22);      // Y
    }
    else {
        // Gimbal lock: pitch is +-90 degrees, z and y are coupled.
        // Choose roll = 0 and compute yaw from remaining terms (one valid choice).
        roll = 0.0f;
        yaw = atan2f(-r20, r00);
    }

    outPitch = pitch;
    outYaw = yaw;
    outRoll = roll;
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

inline void DecomposeTransform(const Mat4& InMatrix, Vec3& OutScale, Vec4& OutQuatRot, Vec3& OutTrans)
{
    XMVECTOR XMScale, XMRotQuat, XMTrans;
    XMMatrixDecompose(&XMScale, &XMRotQuat, &XMTrans, XMLoadFloat4x4(&InMatrix));

    XMRotQuat = XMQuaternionNormalize(XMRotQuat);

    XMStoreFloat3(&OutScale, XMScale);
    XMStoreFloat4(&OutQuatRot, XMRotQuat);
    XMStoreFloat3(&OutTrans, XMTrans);
}

inline void DecomposeTransform(const Mat4& InMatrix, Vec3& OutScale, Vec3& OutRot, Vec3& OutTrans)
{
    XMVECTOR XMScale, XMRotQuat, XMTrans;
    XMMatrixDecompose(&XMScale, &XMRotQuat, &XMTrans, XMLoadFloat4x4(&InMatrix));

    XMTrans /= XMScale;

    Vec4 RotQuat = {};
    XMStoreFloat4(&RotQuat, XMRotQuat);

    XMStoreFloat3(&OutScale, XMScale);
    DXQuaternionToEuler(RotQuat, OutRot.x, OutRot.y, OutRot.z);
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