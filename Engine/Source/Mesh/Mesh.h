#pragma once

#include "Include.h"
#include "Vertex.h"

#include "Core/Asset.h"

struct ENGINE_DLL FMeshData
{
    VertexList		Vertices;
    IndexList 		Indices;

    FMeshData& operator+=(const FMeshData& Rhs)
    {
        this->Vertices.insert(this->Vertices.end(), Rhs.Vertices.begin(), Rhs.Vertices.end());
        this->Indices.insert(this->Indices.end(), Rhs.Indices.begin(), Rhs.Indices.end());

        return *this;
    }

    REFLECT_TOP(FMeshData, PROPERTY(Vertices), PROPERTY(Indices));
};

struct ENGINE_DLL FClothData
{
    uint32 MeshIndex;

    // 각 버텍스들의 역질량
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

    REFLECT_TOP(KeyFrame
        , PROPERTY(Matrices)
    );
};

struct ENGINE_DLL MAnimation : public MAsset
{
    MAnimation()
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
    float			Duration;

public:
    void SetFrameInfo(fbxsdk::FbxTime& InStart, fbxsdk::FbxTime& InEnd);
    // 프레임 단위로 KeyFrame을 가져옴
    KeyFrame& GetKeyFrame(int Frame) { return KeyFrames[Frame]; }
protected:
    // 프레임 단위로 조인트들의 행렬을 저장함. 1프레임 200개행렬, 2프레임 200개행렬 이런 구조.
    std::vector<KeyFrame> KeyFrames;

    REFLECT(MAnimation
        , PROPERTY(Name)
        , PROPERTY(StartFrame)
        , PROPERTY(EndFrame)
        , PROPERTY(TotalFrame)
        , PROPERTY(KeyFrames)
        , PROPERTY(Duration)
    );
};

struct ENGINE_DLL VertexIndexWeightInfo
{
    std::vector<int>		_jointIndexList;
    std::vector<double*>	_weightList;
};

struct ENGINE_DLL FJoint
{
    FJoint()
    {

    }

    // 조인트 이름
    std::string Name;

    // 부모 조인트 인덱스
    int32	_parentIndex = -1;

    // 메시의 점 위치를 조인트 기준의 위치로 변환하는 행렬
    Mat4	_globalBindPoseInverseMatrix = IDENTITYMATRIX;

public:
    // SRT
    Vec3    Scale = {};
    Vec3    Rotation = {};
    Vec3	Position = {};

    REFLECT_TOP(FJoint
        , PROPERTY(Name)
        , PROPERTY(_parentIndex)
        , PROPERTY(_globalBindPoseInverseMatrix)
        , PROPERTY(Scale)
        , PROPERTY(Rotation)
        , PROPERTY(Position)
    );
};

struct ENGINE_DLL FCapsuleData
{
    float HalfHeight = 1.f;
    float Radius = 1.f;

    REFLECT_TOP(
        FCapsuleData
        , PROPERTY(HalfHeight)
        , PROPERTY(Radius)
    );
};

struct ENGINE_DLL FBodyCapsuleData
{
    int32 PrimitiveID = -1;

    int32 AttachJointIndex = -1;
    FCapsuleData CapsuleData;
    Vec3 TranslationOffset = {};
    Vec3 RotationOffset = {};

    float GetRadius()
    {
        return CapsuleData.Radius;
    }

    float GetHalfHeight()
    {
        return CapsuleData.HalfHeight;
    }

    REFLECT_TOP(FBodyCapsuleData
        , PROPERTY(AttachJointIndex)
        , PROPERTY(CapsuleData)
        , PROPERTY(TranslationOffset)
        , PROPERTY(RotationOffset)
    );
};

namespace Mesh
{
    void MakeSphere(FMeshData& OutMeshData, uint32 InSegment);
    void MakeCoordinate(FMeshData& OutMeshData);
    void MakeCapsule(FMeshData& OutMeshData, float InHalfHeight, float InRadius);
    void MakeRect(FMeshData& OutMeshData);
}
