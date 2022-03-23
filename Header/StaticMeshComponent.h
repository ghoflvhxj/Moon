#pragma once
#ifndef __STATIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "Vertex.h"

class ENGINE_DLL StaticMeshComponent : public PrimitiveComponent
{
public:
	explicit StaticMeshComponent();
	explicit StaticMeshComponent(const char *filePathName);
	virtual ~StaticMeshComponent();

private:
	void initializeMeshInformation(const char *filePathName);
private:
	// 서브 셋의 개수에 따라 사이즈가 결정됨
	std::vector<VertexList>		_vertexList;		
	std::vector<IndexList >		_indexList;
	std::vector<TextureList>	_textureList;

public:
	virtual void render() override;
	std::shared_ptr<Material> getMaterial(const uint32 index);
private:
	MaterialList _materialList;

private:
	bool _bNormalTexture;
};

#define __STATIC_MESH_COMPONENT_H__
#endif