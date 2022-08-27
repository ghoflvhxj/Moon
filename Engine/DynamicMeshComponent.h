#pragma once
#ifndef __DYNAMIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "StaticMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Vertex.h"

class Skeleton;

class ENGINE_DLL DynamicMesh : public StaticMesh
{
public:
	DynamicMesh() = default;

public: //삭제예정
	virtual void initializeMeshInformation(const char *filePathName) override;
public:
	AnimationClip& getAnimationClip(const int index);
	const uint32 getJointCount() const;
	std::vector<FJoint>& getJoints();
private:
	std::vector<AnimationClip> _animationClipList;
	std::vector<FJoint> _jointList;
	uint32 _jointCount;

public:
	std::shared_ptr<Skeleton> _pSkeleton = nullptr;
};

class ENGINE_DLL DynamicMeshComponent : public PrimitiveComponent
{
public:
	explicit DynamicMeshComponent();
	explicit DynamicMeshComponent(const char *filePathName);
	virtual ~DynamicMeshComponent();

public:
	virtual const bool getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList) override;

public:
	void playAnimation(const uint32 index, const Time deltaTime);
private:
	uint32 _currentAinmClip = 0;
	float _currentPlayTime = 0.f;
	Mat4 _matrices[200];

public:
	virtual std::shared_ptr<DynamicMesh>& getDynamicMesh();

private:
	std::shared_ptr<DynamicMesh> _pDynamicMesh = nullptr;
};

#define __STATIC_MESH_COMPONENT_H__
#endif