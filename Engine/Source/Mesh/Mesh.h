#pragma once

#include "Include.h"
#include "Vertex.h"

struct ENGINE_DLL FMeshData
{
    VertexList		Vertices;
    IndexList 		Indices;

    //REFLECTABLE(
    //    FMeshData,
    //    REFLECT_FIELD(Vertices),
    //    REFLECT_FIELD(Indices)
    //);

    REFLECT_TOP(FMeshData, PROPERTY(Vertices), PROPERTY(Indices));
};

struct FClothData
{
    uint32 MeshIndex;
    std::vector<float> InvMass;

    // 옷감이 붙는 조인트 인덱스
    int32 JointIndex = -1;

    REFLECT_TOP(FClothData, PROPERTY(MeshIndex), PROPERTY(JointIndex), PROPERTY(InvMass));
};

namespace fbxsdk
{
    class FbxTime;
}

struct ENGINE_DLL KeyFrame
{
    KeyFrame()
        : Matrices(200, IDENTITYMATRIX)
    {
    }
public:
    // 특정 조인트의 행렬을 가져옴
    Mat4& GetJointMatrix(int JointIndex) { return Matrices[JointIndex]; }
protected:
    // 모든 조인트의 행렬을 저장
    std::vector<Mat4> Matrices;
};

struct ENGINE_DLL AnimationClip
{
    AnimationClip()
        : StartFrame{ 0 }
        , EndFrame{ 0 }
        , TotalFrame{ 0 }
        , Duration{ 0.0 }
    {

    }

    std::string		Name;
    uint32			StartFrame;
    uint32			EndFrame;
    uint32			TotalFrame;
    double			Duration;

public:
    void SetFrameInfo(fbxsdk::FbxTime& InStart, fbxsdk::FbxTime& InEnd);
    // 프레임 단위로 KeyFrame을 가져옴
    KeyFrame& GetKeyFrame(int Frame) { return KeyFrames[Frame]; }
protected:
    // 프레임 단위로 조인트들의 행렬을 저장함. 1프레임 200개행렬, 2프레임 200개행렬 이런 구조.
    std::vector<KeyFrame> KeyFrames;
};

struct ENGINE_DLL VertexIndexWeightInfo
{
    std::vector<int>		_jointIndexList;
    std::vector<double*>	_weightList;
};

struct ENGINE_DLL FJoint
{
    FJoint()
        : _parentIndex{ -1 }
        , _position{ 0.f, 0.f, 0.f }
        , _globalBindPoseInverseMatrix{ IDENTITYMATRIX }
    {

    }

    int32	_parentIndex;

    // 조인트에 적용된 바인드 포즈 변환을 지우기 위한 행렬
    Mat4	_globalBindPoseInverseMatrix;
    Vec3	_position;
};

using JointIndexMap = std::unordered_map<std::string, int>;
using VertexWeightInfoListMap = std::unordered_map<int, VertexIndexWeightInfo>;