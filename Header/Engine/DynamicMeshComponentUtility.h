#pragma once
#ifndef __DYNAMICMESH_COMPONENT_UTILITY_H__

#include "Define.h"

struct KeyFrame
{
	KeyFrame()
		: _matrix()
#ifdef _DEBUG
		//, _scale{ VEC3ZERO }
		//, _rotation{ VEC3ZERO }
		//, _translation{ VEC3ZERO }
#endif
	{
	}
	DirectX::XMFLOAT4X4	_matrix;

#ifdef _DEBUG
	//Vec3	_scale;
	//Vec3	_rotation;
	//Vec3	_translation;
#endif
};

struct AnimationClip
{
	using KeyFrameList				= std::vector<std::vector<std::vector<DirectX::XMFLOAT4X4>>>;
	AnimationClip()
		: _startFrame{ 0 }
		, _endFrame{ 0 }
		, _frameCount{ 0 }
		, _duration{ 0.0 }
	{

	}

	std::string		_animationName;
	uint32			_startFrame;
	uint32			_endFrame;
	uint32			_frameCount;
	double			_duration;
	KeyFrameList	_keyFrameLists;
};

struct VertexIndexWeightInfo
{
	std::vector<int>		_jointIndexList;
	std::vector<double*>	_weightList;
};

struct FJoint
{
	FJoint()
		: _name{ "" }
		, _parentIndex{ -1 }
	{

	}

	std::string			_name;
	int32				_parentIndex;
	std::vector<DirectX::XMFLOAT4X4>	_inverseOfGlobalBindPoseMatrices;
};

using JointIndexMap = std::unordered_map<std::string, int>;
using VertexWeightInfoListMap = std::unordered_map<int, VertexIndexWeightInfo>;

#endif __DYNAMICMESH_COMPONENT_UTILITY_H__