#pragma once
#ifndef __DYNAMIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "DynamicMeshComponentUtility.h"

#include "Vertex.h"

class ENGINE_DLL DynamicMeshComponent : public PrimitiveComponent
{
public:
	explicit DynamicMeshComponent();
	explicit DynamicMeshComponent(const char *filePathName);
	virtual ~DynamicMeshComponent();

private:
	void initializeMeshInformation(const char *filePathName);
private:
	// 서브 셋의 개수에 따라 사이즈가 결정됨
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList >		_indicesList;
	std::vector<TextureList>	_texturesList;

public:
	void playAnimation(const uint32 index);
private:
	std::vector<Vec3>			_originVertexPositionList;

public:
	//virtual void render() override;
	std::shared_ptr<Material> getMaterial(const uint32 index);
private:
	MaterialList _materialList;

private:
	std::vector<AnimationClip> _animationClipList;
};

#define __STATIC_MESH_COMPONENT_H__
#endif