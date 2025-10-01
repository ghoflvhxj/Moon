#pragma once

#include "Include.h"
#include "Module/Module.h"
#include "Render.h"

#include "Mesh/Mesh.h" // FCapsuleData 참조용

class SceneComponent;
class StaticMeshComponent;
class MVertexBuffer;
class RenderTarget;
struct FMeshData;
struct FInstancingData
{
    Vec3 Scale = VEC3ONE;
    Vec3 Translation = VEC3ZERO;
    Vec4 Quaternion = IDENTITY3;
};

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
    virtual void Render();
protected:
    void RenderScene();
    void RenderText();
private:
    void UpdateGlobalConstantBuffer();
    void UpdateTickConstantBuffer();

public:
    //void DrawCapsule(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation);
protected:
    //std::vector<FCapsuleRenderData> CapsuleRenderDatas;

public:
    void DrawCylinder(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation);
    void DrawSphere(float InRadius, const Vec3& InTranslation);
    void DrawCoordinate(const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale = VEC3ONE);
protected:
    std::shared_ptr<MMaterial> SphereMaterial = nullptr;
    std::vector<FInstancingData> SphereRenderDatas;
    std::vector<FInstancingData> CoordinateRenderDatas;
    FMeshData SphereMesh = {};
    FMeshData CoordinateMesh = {};
    FMeshData CapsuleMeshData = {};
    uint32 SpherePID = 0;
    uint32 CoordinatePID = 0;

public:
    uint32 DrawVertices(const FMeshData& InMeshData);
    uint32 DrawLine(const std::vector<Vec3>& InWorldPositions);
    //void DrawCapsule(float InRadius, float InHalfHeight);
    uint32 DrawCapsule(float InRadius, float InHalfHeight);
    

public:
	// 렌더할 PrimitiveComponent 추가하는 함수. 렌더 패스에 들어감.
	void AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent);
    // 렌더할 PrimitiveComponent 추가하되, 렌더 패스에 안들어감.
    void AddPrimitiveComponentTemp(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent);
protected:
    // ID, PrimitiveDatas 쌍. 
    std::map<uint32, std::vector<FPrimitiveData>> PrimitiveDatasRenderPass;
    std::map<uint32, std::vector<FPrimitiveData>> PrimitiveDatasNoRenderPass;

public:
    // PrimitiveComponent의 GetPrimitiveData로 데이터를 가져오며, 버텍스 버퍼도 만듬.
    void GetPrimitiveDataFromComponent(std::shared_ptr<MPrimitiveComponent> InComponent);
    // PrimitveData에 버퍼를 설정하는 함수
    void UpdateBuffer(std::shared_ptr<MPrimitiveComponent> InComponent);
    void UpdateBufferInternal(uint32 InPrimitiveID);
    // 버텍스, 인덱스 버퍼를 만드는 함수
    void MakeBuffer(uint32 InPrimitiveID, const FMeshData& InMeshData);
    // 버퍼를 지우는 함수
    void RemoveBuffer(uint32 InPrimitiveID, int32 InIndex = -1);
public:
    const std::vector<FPrimitiveData>& GetPrimitiveDatas(uint32 InPrimitiveID);
protected:
    // 모든 PrimitiveComponent
	std::map<uint32, std::shared_ptr<MPrimitiveComponent>> PrimitiveComponents;

protected:
    // PrimitiveID - 버텍스 버퍼 쌍을 저장함, ConstantBuffer는 쉐이더에서 하는데 모든 버퍼를 Renderer가 관리할지, Shader가 할지 
    std::map<uint32, std::vector<std::shared_ptr<MVertexBuffer>>> VertexBuffers;
    std::map<uint32, std::vector<std::shared_ptr<MIndexBuffer>>> IndexBuffers;

public:
    std::shared_ptr<MVertexBuffer> InstanceBuffer;
    std::shared_ptr<MVertexBuffer> InstanceBuffer2;

public:
    // 컴포넌트 없는 PrimitiveData 업데이트
    void UpdatePrimitiveTransform(uint32 InPrimitiveID, const Vec3& InTranslation, const Vec4& InRotation, const Vec3& InScale);
    void UpdatePrimitiveVertexPos(uint32 InPrimitiveID, const std::vector<Vertex>& InVertices);

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
    // 컬링 등의 과정 후 실제로 렌더링되는 PrimitiveData
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
	uint32 TotalPrimitiveNum = 0;
	uint32 ShownPrimitiveNum = 0;
	uint32 CulledPrimitiveNum = 0;

private:
    std::unordered_map<ERenderTarget, std::shared_ptr<StaticMeshComponent>> DebugRenderTargetMehses;
	bool bDebugRenderTargets = true;

    // TODO. Editor 기능이므로 해당 프로젝트로 옮겨야 함
public:
    bool bGizmo = false;
    Vec3 GizmoPos = VEC3ZERO;
    std::shared_ptr<StaticMeshComponent> GizmoMeshComp = nullptr;

    // Cascade Shadow 구현을 위한 멤버들
protected:
    std::vector<float> CascadeDistance;
    std::vector<Vec4> CascadeLightPositions;
    std::vector<Mat4> CascadeLightMatrices;

    // 공통된 CosntantBuffer
protected:
    

public:
    RENDERER_OPTION(DrawCollision);
    //bool bDrawCollision = false;

    REFLECT(
        MRenderer,
        PROPERTY(bDrawCollision),
        PROPERTY(bDebugRenderTargets)
    )
};
