#pragma once
#include "Include.h"

namespace fbxsdk
{
    class FbxTime;
}

struct KeyFrame
{
public:
    // 특정 조인트의 행렬을 가져옴
    Mat4& GetJointMatrix(int JointIndex) { return Matrices[JointIndex]; }
protected:
    // 모든 조인트의 행렬을 저장
	Mat4 Matrices[200];
};

struct AnimationClip
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

struct VertexIndexWeightInfo
{
	std::vector<int>		_jointIndexList;
	std::vector<double*>	_weightList;
};

struct FJoint
{
	FJoint()
		: _parentIndex{ -1 }
		, _position{ 0.f, 0.f, 0.f }
	{

	}

	int32				_parentIndex;
	Mat4	_globalBindPoseInverseMatrix;
	Vec3	_position;
};

using JointIndexMap = std::unordered_map<std::string, int>;
using VertexWeightInfoListMap = std::unordered_map<int, VertexIndexWeightInfo>;