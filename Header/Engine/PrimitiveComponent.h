#pragma once
#ifndef __PRIMITIVE_COMPONENT_H__

#include "SceneComponent.h"
#include "Vertex.h"

struct PrimitiveData;

class VertexBuffer;
class IndexBuffer;

class ENGINE_DLL BoundingBox
{
public:
	BoundingBox(const Vec3 &min, const Vec3 &max);

	Vec3 _min;
	Vec3 _max;

public:
	virtual const bool Cull(const std::vector<DirectX::XMVECTOR> palnes, const Vec3 &position);
protected:
	std::vector<Vertex>		_vertices;
	std::vector<Index>		_indices;

public:
	std::shared_ptr<VertexBuffer> getVertexBuffer();
	std::shared_ptr<IndexBuffer> getIndexBuffer();
protected:
	std::shared_ptr<VertexBuffer> _pVertexBuffer;
	std::shared_ptr<IndexBuffer> _pIndexBuffer = nullptr;

public:
	std::shared_ptr<Material> getMaterial();
protected:
	std::shared_ptr<Material> _pMaterial;
};

class ENGINE_DLL PrimitiveComponent abstract : public SceneComponent, public std::enable_shared_from_this<PrimitiveComponent>
{
public:
	enum class RenderMode
	{
		Perspective, Orthogonal, End
	};	

public:
	explicit PrimitiveComponent();
	virtual ~PrimitiveComponent();

public:
	virtual void Update(const Time deltaTime) override;

public:
	virtual const bool getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList);
	virtual const bool getBoundingBox(std::shared_ptr<BoundingBox> &boundingBox);

public:
	void				setRenderMode(const RenderMode renderMode);
	const RenderMode	getRenderMdoe() const;
private:
	RenderMode _eRenderMdoe;
};

#define __PRIMITIVE_COMPONENT_H__
#endif