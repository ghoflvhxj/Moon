#pragma once

#include "SceneComponent.h"
#include "Vertex.h"
#include "Core/Delegate.h"

struct FPrimitiveData;
struct FMeshData;

class MVertexBuffer;
class MIndexBuffer;

class ENGINE_DLL MBoundingBox
{
public:
	MBoundingBox(const Vec3 &min, const Vec3 &max);

	Vec3 _min;
	Vec3 _max;

public:
	const bool cull(const std::vector<DirectX::XMVECTOR> palnes, const Vec3 &position);
	const bool cullSphere(const std::vector<DirectX::XMVECTOR> palnes, const Vec3 &position, const float length);
	const float GetLength(const Vec3 &scale = { 1.f, 1.f, 1.f }) const;

protected:
	std::vector<Vertex>		_vertices;
	std::vector<Index>		_indices;

public:
	std::shared_ptr<MMaterial> getMaterial();
protected:
	std::shared_ptr<MMaterial> _pMaterial = nullptr;

public:
    std::shared_ptr<FMeshData> GetMeshData() const;
protected:
    std::vector<std::shared_ptr<FMeshData>> MeshDatas;
};

class ENGINE_DLL MPrimitiveComponent abstract : public SceneComponent
{
public:
	enum class ERenderMode
	{
		Perspective, Orthogonal, End
	};	

public:
	explicit MPrimitiveComponent();
	virtual ~MPrimitiveComponent();

public:
    FDelegate<void, std::shared_ptr<MPrimitiveComponent>>& GetPrimitiveChangedDelegate() { return OnPrimitiveChangedDelegate; }
protected:
    FDelegate<void, std::shared_ptr<MPrimitiveComponent>> OnPrimitiveChangedDelegate;

public:
	const uint32 GetPrimitiveID() const { return PrimitiveID; }
protected:
	static uint32 PrimitiveCounter;
	uint32 PrimitiveID;

public:
	virtual void Update(const Time deltaTime) override;

public:
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList);
	virtual const bool GetBoundingBox(std::shared_ptr<MBoundingBox>& boundingBox);

public:
	void				setRenderMode(const ERenderMode renderMode);
	const ERenderMode	getRenderMdoe() const;
private:
	ERenderMode RenderMode;

public:
	void SetRendering(bool bNewRendering);
    bool IsRendering() const { return bRendering; }
protected:
	bool bRendering = true;

public:
    void setDrawingBoundingBox(const bool bDraw);
    const bool IsDrawingBoundingBox() const;
public:
    bool _bDrawBoundingBox = false;

public:
    bool IsShadowing() const { return bShadowing; }
protected:
    bool bShadowing = true;

public:
    void SetDrawCollision(const bool bDraw) { bDrawColliision = bDraw; }
protected:
    bool bDrawColliision = false;

    // 렌더 데이터가 변경되면 Dirty
public:
    void SetDirty(bool bInDirty) { bDirty = bInDirty; }
    bool IsDirty() const { return bDirty; }
protected:
    bool bDirty = false;

    REFLECT(
        MPrimitiveComponent,
        PROPERTY(RenderMode),
        PROPERTY(bRendering)
    )
};