#pragma once

#include "Include.h"
#include "Module/Module.h"
#include "Render.h"

class SceneComponent;
class StaticMeshComponent;
class MVertexBuffer;
class RenderTarget;

class ENGINE_DLL MRenderer : public MModule
{
public:
	template <class T>
	static std::shared_ptr<T> CreateRenderPass()
	{
		return std::make_shared<T>();
	}

public:
	explicit MRenderer() noexcept;
	virtual ~MRenderer() noexcept;

    // Module의 인터페이스 구현
public:
	virtual bool Initialize();
	virtual void Release();

public:
    void DrawLine(const std::vector<Vec3>& InWorldPositions);
    uint32 MakeCapsule(float InRadius, float InHeight);


public:
	void AddPrimitive(std::shared_ptr<MPrimitiveComponent> pComponent);
public:
	// 렌더할 Primitive 추가
    void MakePrimitiveData(std::shared_ptr<MPrimitiveComponent> InComponent);
    void MakeBuffer(uint32 InPrimitiveID, const FMeshData& InMeshData);
    void MakeBuffer(std::shared_ptr<MPrimitiveComponent> InComponent, std::vector<FPrimitiveData>& InPrimitiveDatas);
    void MakeBuffer(std::shared_ptr<MPrimitiveComponent> InComponent);
protected:
    // 모든 PrimitiveComponent
	std::map<uint32, std::shared_ptr<MPrimitiveComponent>> PrimitiveComponents;
    // PrimitiveID, PrimitiveDatas 쌍
    std::map<uint32, std::vector<FPrimitiveData>> IdToPrimitiveDatas;

    // PrimitiveID - 버텍스 버퍼 쌍을 저장함, ConstantBuffer는 쉐이더에서 하는데 모든 버퍼를 Renderer가 관리할지, Shader가 할지 
    std::map<uint32, std::vector<std::shared_ptr<MVertexBuffer>>> VertexBuffers;
    std::map<uint32, std::vector<std::shared_ptr<MIndexBuffer>>> IndexBuffers;

    // 컴포넌트 없는 PrimitiveData 업데이트
public:
    void UpdatePrimitive(uint32 InPrimitiveID, const Vec3& InTranslation, const Vec4& InRotation, const Vec3& InScale);

    // 라인 테스트
protected:
    std::shared_ptr<MVertexBuffer> V;
    std::shared_ptr<MIndexBuffer> I;

public:
    std::shared_ptr<MVertexBuffer> GetVertexBuffer(uint32 InId, uint32 InOffset = 0);
    std::shared_ptr<MIndexBuffer> GetIndexBuffer(uint32 InId, uint32 InOffset = 0);

public:
    const std::vector<FPrimitiveData>& GetPrimitives(EPrimitiveType InPrimitiveType) { return PrimitiveDatasPerType[InPrimitiveType]; }
    // PrimitiveType - PrimitiveData 쌍을 저장함
    std::map<EPrimitiveType, std::vector<FPrimitiveData>> PrimitiveDatasPerType;

private:
    void FrustumCulling();
public:
    const std::vector<FPrimitiveData>& GetRenderablePrimitiveData() const { return RenderablePrimitiveData; }
protected:
    // 컬링 후 남은 PrimitiveData
    std::vector<FPrimitiveData> RenderablePrimitiveData;

	// 렌더 타겟
public:
	void addRenderTargetForDebug(ERenderTarget InRenderTarget);
	std::shared_ptr<RenderTarget>& GetRenderTarget(ERenderTarget RenderTarget) { return _renderTargets[static_cast<int32>(RenderTarget)]; }
private:
	RenderTargets _renderTargets;

	// 렌더 패스
private:
	std::vector<std::shared_ptr<MRenderPass>> RenderPasses;

public:
	void Render();
	void RenderScene();
	void RenderText();
private:
	void UpdateGlobalConstantBuffer(std::shared_ptr<MShader>& Shader);
    void UpdateTickConstantBuffer(std::shared_ptr<MShader>& Shader);

public:
	uint32 TotalPrimitiveNum = 0;
	uint32 ShownPrimitiveNum = 0;
	uint32 CulledPrimitiveNum = 0;

private:
    std::unordered_map<ERenderTarget, std::shared_ptr<StaticMeshComponent>> DebugRenderTargetMehses;
	bool bDebugRenderTargets = true;

// Editor
public:
    bool bGizmo = false;
    Vec3 GizmoPos = VEC3ZERO;
    std::shared_ptr<StaticMeshComponent> GizmoMeshComp = nullptr;

protected:
    std::vector<float> _cascadeDistance;
    std::vector<Vec3> LightPosition;
    std::vector<Mat4> LightViewProj;

public:
    RENDERER_OPTION(DrawCollision);
    //bool bDrawCollision = false;

    REFLECT(
        MRenderer,
        PROPERTY(bDrawCollision),
        PROPERTY(bDebugRenderTargets)
    )
};
