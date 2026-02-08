#pragma once

#include "Include.h"
#include "Render.h"

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

    /*********************************
        HLSL 글로벌 파라미터
    **********************************/
public:
    float NormalBiasScale = 0.1f;
    float DepthBias = 0.001f;
    bool bDebugDirectLight = false;
    bool bDebugInDirectLight = false;
    bool bDebugDirectionalShadow = false;
    bool bDebugCascade = false;
    bool bDebugSSAO = false;

    /* 렌더링에 필요한 PrimitiveData를 관리함 */
public:
    // PrimitveData에 버퍼를 설정하는 함수
    void UpdateBuffersFromMesh(uint32 InPID, std::shared_ptr<MMesh>& InMesh);
    void UpdateBuffersFromShader(uint32 InPID, std::shared_ptr<MShader> InShader);
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
    void AddRenderablePrimitiveData(const FPrimitiveData& InPrimitiveData);
    std::vector<FPrimitiveData>& GetRenderablePrimitiveData();
protected:
    // 컬링 후 실제로 렌더링되는 PrimitiveData를 저장함
    std::vector<FPrimitiveData> RenderablePrimitiveData;

    /*********************************
        Temporal Primitive Data 관리
        PID가 없는 경우에 사용, 휘발성이니 매 프레임 추가 해줘야 함.
    **********************************/
public:
    void AddTemporalPrimitiveData(const FPrimitiveData& InPrimitiveData);
public:
    const std::vector<FPrimitiveData>& GetTemporalPrimitiveDatas() const;
protected:
    std::vector<FPrimitiveData> TemporalPrimitiveDatas;
    // 피킹 처리를 위해 있는데, 제거할 방법을 생각해봐야 함
    std::vector<FPrimitiveData> CachedTemporalPrimitiveDatas;

    /*********************************
        Cascade Shadow 구현
    **********************************/
public:
    const std::vector<float>& GetCascadeDistances() const { return CascadeDistances; }
    float GetCascadeDistance(uint32 InIndex) const { return CascadeDistances[InIndex]; }
protected:
    std::vector<float> CascadeDistances;

    /*********************************
        인스턴싱
    *********************************/
public:
    void AddInstanceData(MObject* InDataOwner, uint32 InSize);
    void AddTemporalInstanceData(const std::wstring& InKey, FPrimitiveData& InPrimitiveData, const std::shared_ptr<MVertexBuffer> InInstanceBuffer);
public:
    std::map<MObject*, std::vector<FVertex_Instance>> InstacingDatas;
    // 인스턴싱 데이터. BufferID - InstanceDatas 쌍
    std::map<std::wstring, std::vector<FVertex_Instance>> TemporalInstanceDatas;
    std::map<std::wstring, std::weak_ptr<MVertexBuffer>> TemporalInstanceBuffers;
    // 인스턴싱 데이터. 재사용 인스턴싱. 
    //std::vector<FVertex_Instance> RecycleInstaincDatas;

public:
    RENDERER_OPTION(DrawCollision, false);

    REFLECT(
        MScene
        , PROPERTY(bDrawCollision)
        , PROPERTY(NormalBiasScale)
        , PROPERTY(DepthBias)
        , PROPERTY(bDebugDirectLight)
        , PROPERTY(bDebugInDirectLight)
        , PROPERTY(bDebugDirectionalShadow)
        , PROPERTY(bDebugCascade)
        , PROPERTY(bDebugSSAO)
    )
        /* 카메라 */
    //public:
    //    void Func(); // 카메라의 데이터를 가져옴
    //protected:
    //    Mat4 ViewPerspectiveProjMatrix = {};
    //    Mat4 ViewOrthogonalProjMatrix = {};
};