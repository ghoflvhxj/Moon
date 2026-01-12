#pragma once

#include "Include.h"
#include "Module/Module.h"
#include "Render.h"

#include "Mesh/Mesh.h" // FCapsuleData 참조용

class MSceneComponent;
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

struct FRenderTargetDebugData
{
    uint32 Index = 0;
    std::shared_ptr<MMaterial> Material = nullptr;
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
        Module의 인터페이스
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
    void DrawCapsule(MWorld* InWorld, float InRadius, float InHalfHeight, const Vec3& InTranslation, const Vec3& InRotation);
    void DrawCapsule(MWorld* InWorld, float InRadius, float InHalfHeight, const Vec3& InTranslation, const Vec4& InQuatRotation);
    void DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec4& InQuatRotation, const Vec3& InScale = VEC3ONE);
    void DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale = VEC3ONE);
    void DrawPrimitive(MWorld* InWorld, const std::shared_ptr<StaticMesh>& InMesh, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale, EPrimitiveType InPrimitiveType);
protected:
    FMeshData SphereMesh = {};
    FMeshData CoordinateMesh = {};
    std::map<std::tuple<int, int>, FMeshData> CapsuleMeshDatas;

public:
    //uint32 DrawVertices(const FMeshData& InMeshData);
    //uint32 DrawLine(const std::vector<Vec3>& InWorldPositions);
    //uint32 DrawCapsule(float InRadius, float InHalfHeight);

public:
    void Test(uint32 InWorldID, uint32 InPID, std::shared_ptr<MMesh> InMesh);
    
public:
    MScene* GetCurrentScene();
    MScene* GetScene(uint32 InWorldID);
protected:
    std::map<uint32, std::unique_ptr<MScene>> Scenes;
    std::map<uint32, std::unique_ptr<MScene>> ScenesQueue;

	// 렌더할 PrimitiveComponent 추가하는 함수. 렌더 패스에 들어감.
public:
	void AddPrimitiveComponent(std::shared_ptr<MPrimitiveComponent> InPrimitiveComponent);
    void RemovePrimitiveComponent(MPrimitiveComponent* InComponent);

    void UpdatePrimitiveData(MPrimitiveComponent* InComponent);

public:
    // 현재 그리는 씬에서 PrimitiveData를 얻어옴
    const std::vector<const FPrimitiveData*>& GetPrimitiveDatas(EPrimitiveType InPrimitiveType);

private:
    void FrustumCulling(std::unique_ptr<MScene>& InScene);

    /**********************************
        렌더 타겟 관리
    **********************************/
public:
    void AddRenderTargets(uint32 InWidth, uint32 InHeight);
    void ResizeRenderTargets(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight, bool InFullScreen);
	void DebugRenderTarget(ERenderTarget InRenderTarget);
public:
    std::shared_ptr<MRenderTarget> GetRenderTarget(ERenderTarget InRenderTarget);
    ID3D11ShaderResourceView* GetResourceView(ERenderTarget InRenderTarget);
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
    std::unordered_map<ERenderTarget, FRenderTargetDebugData> DebugRenderTargetData;
	int DebugRenderTargetIndex = -1;

public:
    Mat4 ViewPerspectiveProjMatrix = {};
    Mat4 ViewOrthogonalProjMatrix = {};
public:
    Vec3 Ambient = VEC3ONE;

public:
    std::vector<std::wstring> Times;
    std::wstring SceneRenderTime;
    std::wstring RenderPassTime;

    RENDERER_OPTION(DrawShadow);

    REFLECT(
        MRenderer
        , PROPERTY(DebugRenderTargetIndex)
        , PROPERTY(Ambient)
        , PROPERTY(bDrawShadow)
    )
};

// Scene과 World는 한쌍으로 존재함.
// Scene은 Render모듈에서의 World라고 이해하면 편함
class ENGINE_DLL MScene : public MObject
{
public:
    MScene();

public:
    // 이름 임시
    void Begin();
    void End();
    void Clear()
    {
        PrimitiveDatas.clear();
        PrimitiveDatasPerType.clear();
    }

public:
    std::shared_ptr<MWindow> GetWindow() const;

public:
    void SetWorld(std::shared_ptr<MWorld> InWorld);
    std::shared_ptr<MWorld> GetWorld() const;
protected:
    std::weak_ptr<MWorld> World;

/**********************************
HLSL ConstantBuffer 업데이트
**********************************/
public:
    void UpdateGlobalConstantBuffer();
    void UpdateTickConstantBuffer();


    /* 렌더링에 필요한 PrimitiveData를 관리함 */
public:
    // PrimitveData에 버퍼를 설정하는 함수
    void UpdateBuffer(uint32 InPID, std::shared_ptr<MMesh>& InMesh);
    void UpdatePrimitiveData(MPrimitiveComponent* InComponent);
public:
    uint32 GetPrimitiveDataNum(uint32 InPID) const { return PrimitiveDatas.find(InPID) != PrimitiveDatas.end() ? GetSize(PrimitiveDatas.at(InPID)) : 0; }
    void AddPrimitiveDatas(uint32 InPID, const std::vector<FPrimitiveData>& InPrimitiveDatas);
    void ClearPrimtiveDatas(uint32 InPID);
    const std::map<uint32, std::vector<FPrimitiveData>>& GetPrimitiveDatas() const;
    const std::vector<FPrimitiveData>& GetPrimitiveDatas(uint32 InPrimitiveID);
    const std::vector<const FPrimitiveData*>& GetPrimitiveDatas(EPrimitiveType InPrimitiveType) { return PrimitiveDatasPerType[InPrimitiveType]; }
protected:
    // PrimitiveComponent로 부터 얻어낸 PrimitiveID, PrimitiveData 쌍을 저장함
    std::map<uint32, std::vector<FPrimitiveData>> PrimitiveDatas;
    std::map<EPrimitiveType, std::vector<const FPrimitiveData*>> PrimitiveDatasPerType;

public:
    void AddRenderablePrimitiveDatas(const std::vector<FPrimitiveData>& InPrimitiveDatas);
    const std::vector<FPrimitiveData>& GetRenderablePrimitiveData() const;
protected:
    // 컬링 후 실제로 렌더링되는 PrimitiveData를 저장함
    std::vector<FPrimitiveData> RenderablePrimitiveData;

public:


public:
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
public:

    // HLSL 글로벌 파라미터
    float NormalBiasScale = 0.1f;
    float DepthBias = 0.001f;
    bool bDebugDirectionalLight = false;
    bool bDebugDirectionalShadow = false;
    bool bDebugCascade = false;

public:
    // 인스턴싱 데이터
    std::map<MVertexBuffer*, std::vector<FVertex_Instance>> InstanceDatas;

public:
    RENDERER_OPTION(DrawCollision);

    REFLECT(
        MScene
        , PROPERTY(bDrawCollision)
        , PROPERTY(NormalBiasScale)
        , PROPERTY(DepthBias)
        , PROPERTY(bDebugDirectionalLight)
        , PROPERTY(bDebugDirectionalShadow)
        , PROPERTY(bDebugCascade)
    )
    /* 카메라 */
//public:
//    void Func(); // 카메라의 데이터를 가져옴
//protected:
//    Mat4 ViewPerspectiveProjMatrix = {};
//    Mat4 ViewOrthogonalProjMatrix = {};
};