#pragma once

#include "SceneComponent.h"
#include "Vertex.h"

struct FPrimitiveData;
struct FMeshData;

class MVertexBuffer;
class MIndexBuffer;

class ENGINE_DLL BoundingBox
{
public:
	BoundingBox(const Vec3 &min, const Vec3 &max);

	Vec3 _min;
	Vec3 _max;

public:
	const bool cull(const std::vector<DirectX::XMVECTOR> palnes, const Vec3 &position);
	const bool cullSphere(const std::vector<DirectX::XMVECTOR> palnes, const Vec3 &position, const float length);
	const float getLength(const Vec3 &scale = { 1.f, 1.f, 1.f }) const;

protected:
	std::vector<Vertex>		_vertices;
	std::vector<Index>		_indices;

public:
	std::shared_ptr<MVertexBuffer> getVertexBuffer();
	std::shared_ptr<MIndexBuffer> getIndexBuffer();
protected:
	std::shared_ptr<MVertexBuffer> _pVertexBuffer;
	std::shared_ptr<MIndexBuffer> _pIndexBuffer = nullptr;

public:
	std::shared_ptr<MMaterial> getMaterial();
protected:
	std::shared_ptr<MMaterial> _pMaterial = nullptr;

public:
    const std::shared_ptr<FMeshData>& GetMeshData() const { return MeshData; }
protected:
    std::shared_ptr<FMeshData> MeshData = nullptr;
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
	const uint32 GetPrimitiveID() const { return PrimitiveID; }
protected:
	static uint32 PrimitiveCounter;
	uint32 PrimitiveID;

public:
	virtual void Update(const Time deltaTime) override;

public:
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList);
	virtual const bool getBoundingBox(std::shared_ptr<BoundingBox> &boundingBox);

public:
	void				setRenderMode(const RenderMode renderMode);
	const RenderMode	getRenderMdoe() const;
private:
	RenderMode _eRenderMdoe;

public:
	void SetRendering(bool bNewRendering);
protected:
	bool bRendering;

public:
    void setDrawingBoundingBox(const bool bDraw);
    const bool IsDrawingBoundingBox() const;
public:
    bool _bDrawBoundingBox = false;
};