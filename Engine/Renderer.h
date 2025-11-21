#pragma once

#include "Include.h"
#include "Module/Module.h"
#include "Render.h"

#include "Mesh/Mesh.h" // FCapsuleData 참조용

class SceneComponent;
class StaticMeshComponent;
class MVertexBuffer;
class MRenderTarget;
class MWorld;
class MScene;

struct FMeshData;
struct FWorldRenderInfo;

struct FInstancingData
{
    Vec3 Scale = VEC3ONE;
    Vec3 Translation = VEC3ZERO;
    Vec4 RotationQuat = IDENTITY3;
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
    MRenderer(const MRenderer& Rhs) = delete;
    virtual ~MRenderer() noexcept;

public:
    MRenderer& operator=(const MRenderer& Rhs) = delete;

    /********************************** 
        Module의 인터페이스 구현
    **********************************/
public:
    virtual bool Initialize() override;
    virtual void Release() override;
    virtual void Render() override;

    /**********************************
        Scene 관리
    **********************************/
public:
    void RenderWorld(const std::shared_ptr<MWorld>& InWorld);
    std::shared_ptr<MWorld> GetWorld();
protected:
    void AddScene(const FWorldRenderInfo& InWorldRenderInfo);
    void RenderScene(std::unique_ptr<MScene>& InScene);
    void RenderText();
    uint32 CurrentSceneID = 0;

    /**********************************
        간단한 Geometry 렌더링
    **********************************/
public:
    void DrawCylinder(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation);
    void DrawSphere(float InRadius, const Vec3& InTranslation, const DirectX::XMVECTORF32& InColor = EngineColors::White);
    void DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale = VEC3ONE);
    void DrawPrimitive(MWorld* InWorld, std::shared_ptr<StaticMesh>& InMesh, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale, EPrimitiveType InPrimitiveType);
protected:
    std::shared_ptr<MMaterial> SphereMaterial = nullptr;
    std::vector<FInstancingData> SphereRenderDatas;
    std::vector<FInstancingData> CoordinateRenderDatas;
    FMeshData SphereMesh = {};
    FMeshData CoordinateMesh = {};
    FMeshData CapsuleMeshData = {};
public:
    uint32 SpherePID = 0;
    uint32 CoordinatePID = 0;

public:
    //uint32 DrawVertices(const FMeshData& InMeshData);
    //uint32 DrawLine(const std::vector<Vec3>& InWorldPositions);
    //void DrawCapsule(float InRadius, float InHalfHeight);
    //uint32 DrawCapsule(float InRadius, float InHalfHeight);
    
public:
    MScene* GetCurrentScene();
    MScene* GetScene(uint32 InWorldID);
protected:
    std::map<uint32, std::unique_ptr<MScene>> Scenes;

	// 렌더할 PrimitiveComponent 추가하는 함수. 렌더 패스에 들어감.
public:
	void AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent);
protected:
    std::map<uint32, std::vector<FPrimitiveData>> PrimitiveDatas;

protected:
    // 모든 PrimitiveComponent
	std::map<uint32, std::shared_ptr<MPrimitiveComponent>> PrimitiveComponents;

public:
    // TODO. 인스턴싱 임시 작업으로 제거해야 됨
    std::shared_ptr<MVertexBuffer> InstanceBuffer;
    std::shared_ptr<MVertexBuffer> InstanceBuffer2;

public:
    // 컴포넌트 없는 PrimitiveData 업데이트
    //void UpdatePrimitiveTransform(uint32 InPrimitiveID, const Vec3& InTranslation, const Vec4& InRotation, const Vec3& InScale);
    //void UpdatePrimitiveVertexPos(uint32 InPrimitiveID, const std::vector<Vertex>& InVertices);

public:
    // 현재 그리는 씬에서 PrimitiveData를 얻어옴
    const std::vector<FPrimitiveData>& GetPrimitives(EPrimitiveType InPrimitiveType);
    // PrimitiveType - PrimitiveData 쌍을 저장함
    std::map<EPrimitiveType, std::vector<FPrimitiveData>> PrimitiveDatasPerType;

private:
    void FrustumCulling(std::unique_ptr<MScene>& InScene);

    /**********************************
        렌더 타겟 관리
    **********************************/
public:
    void AddRenderTargets(uint32 InWidth, uint32 InHeight);
    void ResizeRenderTargets(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight);
	void DebugRenderTarget(ERenderTarget InRenderTarget);
public:
    std::shared_ptr<MRenderTarget> GetRenderTarget(ERenderTarget InRenderTarget);
private:
	//RenderTargets _renderTargets;
    std::map<std::tuple<uint32, uint32>, RenderTargets> RenderTargetss;

	// 렌더 패스
public:
    void AddRenderPass(ERenderPass InRenderPassIndex, const std::shared_ptr<MRenderPass>& InRenderPass);
private:
	std::vector<std::shared_ptr<MRenderPass>> RenderPasses;

public:
	uint32 TotalPrimitiveNum = 0;
	uint32 ShownPrimitiveNum = 0;
	uint32 CulledPrimitiveNum = 0;

private:
    std::unordered_map<ERenderTarget, std::shared_ptr<StaticMeshComponent>> DebugRenderTargetMehses;
	bool bDebugRenderTargets = true;

    // Cascade Shadow 구현을 위한 멤버들
protected:
    //std::vector<float> CascadeDistances;
    //std::vector<Vec4> CascadeLightPositions;
    //std::vector<Mat4> CascadeLightMatrices;

public:
    Mat4 ViewPerspectiveProjMatrix = {};
    Mat4 ViewOrthogonalProjMatrix = {};
public:
    RENDERER_OPTION(DrawCollision);
    //bool bDrawCollision = false;
    Vec3 Ambient = VEC3ONE;

    REFLECT(
        MRenderer
        , PROPERTY(bDrawCollision)
        , PROPERTY(bDebugRenderTargets)
        , PROPERTY(Ambient)
    )
};

// Scene과 World는 한쌍으로 존재함.
// Scene은 Render모듈에서의 World라고 이해하면 편함
class ENGINE_DLL MScene
{
public:
    MScene();

public:
    // 이름 임시
    void Begin();
    void End();

public:
    std::shared_ptr<MWindow> GetWindow() const;

public:
    void SetWorld(std::shared_ptr<MWorld> InWorld) { World = InWorld; }
    std::shared_ptr<MWorld> GetWorld() const;
protected:
    std::weak_ptr<MWorld> World;

public:
    void UpdateGlobalConstantBuffer();
    void UpdateTickConstantBuffer();

    /**********************************
        HLSL ConstantBuffer 업데이트
    **********************************/
public:
    void MakeSpherePrimitives();
    void MakeCoordinatePrimitives();

    /* 렌더링에 필요한 PrimitiveData를 관리함 */
public:
    // 렌더할 PrimitiveComponent 추가하는 함수.렌더 패스에 들어감.
    void AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent>& InPrimitiveComponent, std::shared_ptr<MMesh>& InMesh);
    // PrimitiveData를 Component로부터 추출해 오는 함수. 버텍스 버퍼도 만듬.
    void GetPrimitiveDataFromComponent(std::shared_ptr<MPrimitiveComponent> InComponent, std::shared_ptr<MMesh>& InMesh);
    // PrimitveData에 버퍼를 설정하는 함수
    void UpdateBuffer(uint32 InPID, std::shared_ptr<MMesh>& InMesh);
    void UpdatePrimtiveData(std::shared_ptr<MPrimitiveComponent> InComponent);
public:
    void AddPrimitiveDatas(uint32 InPID, std::vector<FPrimitiveData> InPrimitiveDatas);
    void ClearPrimtiveDatas(uint32 InPID);
    const std::vector<FPrimitiveData>& GetPrimitiveDatas(uint32 InPrimitiveID);
    const std::map<uint32, std::vector<FPrimitiveData>>& GetPrimitiveDatas() const;
protected:
    // PrimitiveComponent로 부터 얻어낸 PrimitiveID, PrimitiveData 쌍을 저장함
    std::map<uint32, std::vector<FPrimitiveData>> PrimitiveDatas;

public:
    void AddRenderablePrimitiveDatas(const std::vector<FPrimitiveData>& InPrimitiveDatas);
    const std::vector<FPrimitiveData>& GetRenderablePrimitiveData() const;
protected:
    // 컬링 후 실제로 렌더링되는 PrimitiveData를 저장함
    std::vector<FPrimitiveData> RenderablePrimitiveData;

public:
    const std::vector<FPrimitiveData>& GetPrimitives(EPrimitiveType InPrimitiveType) { return PrimitiveDatasPerType[InPrimitiveType]; }
protected:
    // 모든 PrimitiveComponent
    std::map<uint32, std::shared_ptr<MPrimitiveComponent>> PrimitiveComponents;
    // PrimitiveType - PrimitiveData 쌍을 저장함
    std::map<EPrimitiveType, std::vector<FPrimitiveData>> PrimitiveDatasPerType;

public:
    void DrawSphere(float InRadius, const Vec3& InTranslation, const DirectX::XMVECTORF32& InColor = EngineColors::White);
    void DrawCoordinate(const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale = VEC3ONE);
    void DrawPrimitive(const FPrimitiveData& InPrimitiveData);
public:
    const std::vector<FPrimitiveData>& GetTemporalPrimitiveDatas() const;
protected:
    // 휘발성 PrimitiveData. 현재 틱이 끝나면 클리어됨
    std::vector<FPrimitiveData> TemporalPrimitiveDatas;
    std::vector<FPrimitiveData> CachedTemporalPrimitiveDatas;

protected:
    std::vector<FInstancingData> SphereRenderDatas;
    std::vector<FInstancingData> CoordinateRenderDatas;

    /*********************************
        Cascade Shadow 구현
    **********************************/
public:
    float GetCascadeDistance(uint32 InIndex) const { return CascadeDistances[InIndex]; }
    void SetLightInfoCascadeShadow(uint32 InIndex, const XMVECTOR& InXMLightPos, const XMMATRIX& InXMLightMat);
protected:
    std::vector<float> CascadeDistances;
    std::vector<Vec4> CascadeLightPositions;
    std::vector<Mat4> CascadeLightMatrices;

    /* 카메라 */
//public:
//    void Func(); // 카메라의 데이터를 가져옴
//protected:
//    Mat4 ViewPerspectiveProjMatrix = {};
//    Mat4 ViewOrthogonalProjMatrix = {};
};