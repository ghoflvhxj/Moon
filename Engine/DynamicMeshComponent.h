#pragma once
#ifndef __DYNAMIC_MESH_COMPONENT_H__
#include "PrimitiveComponent.h"

#include "StaticMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Vertex.h"

class ENGINE_DLL DynamicMesh : public StaticMesh
{
public:
	explicit DynamicMesh() = default;
};

class ENGINE_DLL DynamicMeshComponent : public PrimitiveComponent
{
public:
	explicit DynamicMeshComponent();
	explicit DynamicMeshComponent(const char *filePathName);
	virtual ~DynamicMeshComponent();

public:
	virtual const bool getPrimitiveData(PrimitiveData &primitiveData) override;

public:
	void DynamicMeshComponent::playAnimation(const uint32 index);

public:
	virtual std::shared_ptr<DynamicMesh>& getDynamicMesh();

private:
	std::shared_ptr<DynamicMesh> _pDynamicMesh = nullptr;
};

#define __STATIC_MESH_COMPONENT_H__
#endif