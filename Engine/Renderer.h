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
        Geometry 렌더링
        PrimitiveComponent 없이 Mesh를 직접 전달할 때 사용
    **********************************/
public:
    void DrawPrimitive(MWorld* InWorld, const std::shared_ptr<StaticMesh>& InMesh, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale, EPrimitiveType InPrimitiveType);
public:
    void DrawCylinder(float InRadius, float InHalfHeight, Vec3& InRotation, Vec3& InTranslation);
    void DrawSphere(float InRadius, const Vec3& InTranslation, const DirectX::XMVECTORF32& InColor = EngineColors::White);
    void DrawCapsule(MWorld* InWorld, float InRadius, float InHalfHeight, const Vec3& InTranslation, const Vec3& InRotation);
    void DrawCapsule(MWorld* InWorld, float InRadius, float InHalfHeight, const Vec3& InTranslation, const Vec4& InQuatRotation);
    void DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec4& InQuatRotation, const Vec3& InScale = VEC3ONE);
    void DrawCoordinate(MWorld* InWorld, const Vec3& InTranslation, const Vec3& InRotation, const Vec3& InScale = VEC3ONE);
protected:
    FMeshData SphereMesh = {};
    FMeshData CoordinateMesh = {};
    std::map<std::tuple<int, int>, FMeshData> CapsuleMeshDatas;

public:
    //uint32 DrawVertices(const FMeshData& InMeshData);
    //uint32 DrawLine(const std::vector<Vec3>& InWorldPositions);
    //uint32 DrawCapsule(float InRadius, float InHalfHeight);

    /***************************************
        업다운 샘플링 기능, 이미시브 개발 중에 임시로 만들었으나 남겨둠
    ***************************************/
public:
    //std::shared_ptr<MRenderTarget> UpDownSampling(std::shared_ptr<MRenderTarget> InRenderTarget, uint32 InWidth, uint32 InHeight);
    //std::shared_ptr<MRenderTarget> Blur(std::shared_ptr<MRenderTarget> InRenderTarget);
    //std::shared_ptr<MRenderTarget> SamplingRenderTarget;
    //FMeshData MeshData;

    /***************************************
        블러 기능
    ***************************************/
public:
    std::vector<float> MakeGaussianWeights(int InRadius, float InSigma);
protected:
    float GaussianSigma = 2.0f;
    int GaussianRadius = 5;
    std::vector<float> Weights;
    MStructuredBuffer Buffer;
    
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
protected:
    void UpdatePrimitiveData(MPrimitiveComponent* InComponent);

public:
    // 현재 그리는 씬에서 PrimitiveData를 얻어옴
    // 포인터 말고 인덱스로 변경하는게 좋을듯
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
    void DebugRenderTarget(std::shared_ptr<MRenderTarget> InRederTarget, const Vec3& InTrans);
public:
    std::shared_ptr<MRenderTarget> GetRenderTarget(ERenderTarget InRenderTarget);
    ID3D11ShaderResourceView* GetResourceView(ERenderTarget InRenderTarget);
private:
	//RenderTargets _renderTargets;
    std::map<std::tuple<uint32, uint32>, RenderTargets> RenderTargetss;

	// 렌더 패스
public:
    void AddRenderPass(ERenderPass InRenderPassIndex, const std::shared_ptr<MRenderPass>& InRenderPass);
    std::shared_ptr<MRenderPass> GetRenderPass(ERenderPass InRenderPassIndex);
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
    std::vector<std::wstring> Times;
    std::wstring SceneRenderTime;
    std::wstring RenderPassTime;

    RENDERER_OPTION(DirectionalLighting, true);
    RENDERER_OPTION(PointLighting, true);
    RENDERER_OPTION(Shadowing, true);
    RENDERER_OPTION(SSAO, true);

    REFLECT(
        MRenderer
        , PROPERTY(DebugRenderTargetIndex)
        , PROPERTY(GaussianSigma)
        , PROPERTY(GaussianRadius)
        , PROPERTY(bDirectionalLighting)
        , PROPERTY(bPointLighting)
        , PROPERTY(bShadowing)
        , PROPERTY(bSSAO)
    )
};