#include "Scene.h"

#include "MoonEngine.h"
#include "World.h"
#include "Camera.h"

#include "PrimitiveComponent.h"
#include "MeshComponent.h"
#include "Framework/Component/FX/FXComponent.h"

MScene::MScene()
    : CascadeDistances(4, 0.f)
{
    CascadeDistances[CastValue<int>(ECascade::Near)] = 0.1f;
    CascadeDistances[CastValue<int>(ECascade::Middle)] = 6.f;
    CascadeDistances[CastValue<int>(ECascade::Middle2)] = 30.f;
    CascadeDistances[CastValue<int>(ECascade::Far)] = 100.f;
}

void MScene::Begin()
{
    RenderablePrimitiveData.clear();
    PrimitiveDatasPerType.clear();

    // 패스마다 매번 할 필요는 없고, 패스가 시작되기 전에 해주면 될듯
    for (auto& [Key, InstanceDatas] : TemporalInstanceDatas)
    {
        TemporalInstanceBuffers[Key].lock()->Update(InstanceDatas.data(), GetSize(InstanceDatas));
    }

    for (auto& [PID, PrimitiveDataList] : PrimitiveDatas)
    {
        for (auto& PrimitiveData : PrimitiveDataList)
        {
            PrimitiveDatasPerType[PrimitiveData.PrimitiveType].push_back(&PrimitiveData);
        }
    }

    UpdateGlobalConstantBuffer();
    UpdateTickConstantBuffer();
}

void MScene::End()
{
    CachedTemporalPrimitiveDatas = std::move(TemporalPrimitiveDatas);

    TemporalInstanceBuffers.clear();
    TemporalInstanceDatas.clear();
}

std::shared_ptr<MWindow> MScene::GetWindow() const
{
    return GetEngine()->GetWorldBoundedWindow(GetWorld().get());
}

void MScene::SetWorld(std::shared_ptr<MWorld> InWorld)
{
    World = InWorld;
}

std::shared_ptr<MWorld> MScene::GetWorld() const
{
    return World.lock();
}

void MScene::UpdateGlobalConstantBuffer()
{
    std::shared_ptr<MConstantBuffer>& GlobalBuffer = MShader::GetSharedConstantBuffer(EConstantBufferLayer::Global);
    if (GlobalBuffer == nullptr)
    {
        return;
    }

    Vec4 resolution = { GetWindow()->GetWidth<float>(),GetWindow()->GetHeight<float>(), 0.f, 0.f };
    GlobalBuffer->SetData(TEXT("resolution"), &resolution);

    BOOL bLight = TRUE;
    GlobalBuffer->SetData(TEXT("bLight"), &bLight);

    // 기타 옵션들 자동으로 설정
    for (auto& Prop : GetTypeDesc()->Properties)
    {
        if (Prop->Type == EType::Bool)
        {
            bool bValue = *static_cast<bool*>(Prop->GetAsVoid(this));
            BOOL Value = bValue ? TRUE : FALSE;
            GlobalBuffer->SetData(StringToWString(Prop->Name), &Value);
        }
        else
        {
            GlobalBuffer->SetData(StringToWString(Prop->Name), Prop->GetAsVoid(this));
        }
    }

    getGraphicDevice()->SetGlboalConstantBuffer(GlobalBuffer);
}

void MScene::UpdateTickConstantBuffer()
{
    EConstantBufferLayer Layer = EConstantBufferLayer::Tick;
    uint32 LayerIndex = EnumToIndex(Layer);
    std::shared_ptr<MConstantBuffer>& TickBuffer = MShader::GetSharedConstantBuffer(Layer);
    if (TickBuffer == nullptr)
    {
        return;
    }

    auto& Camera = GetWorld()->getMainCamera();
    if (Camera == nullptr)
    {
        return;
    }

    TickBuffer->SetData(TEXT("viewMatrix"), &Camera->getViewMatrix());
    TickBuffer->SetData(TEXT("InverseViewMatrix"), &Camera->getInvesrViewMatrix());

    TickBuffer->SetData(TEXT("projectionMatrix"), &Camera->getProjectionMatrix());
    TickBuffer->SetData(TEXT("InverseProjectionMatrix"), &Camera->getInverseProjectionMatrix());
    TickBuffer->SetData(TEXT("orthographicProjectionMatrix"), &Camera->getOrthographicProjectionMatrix());
    TickBuffer->SetData(TEXT("inverseOrthographicProjectionMatrix"), &Camera->getInverseOrthographicProjectionMatrix());

    XMMATRIX XMViewProjMat = XMLoadFloat4x4(&Camera->getViewMatrix()) * XMLoadFloat4x4(&Camera->getProjectionMatrix());
    Mat4 ViewProjMat = {};
    XMStoreFloat4x4(&ViewProjMat, XMViewProjMat);
    TickBuffer->SetData(TEXT("ViewProjMatrix"), &ViewProjMat);

    Mat4 InvViewProjMat = {};
    XMStoreFloat4x4(&InvViewProjMat, XMMatrixInverse(nullptr, XMViewProjMat));
    TickBuffer->SetData(TEXT("InvViewProjMatrix"), &InvViewProjMat);

    TickBuffer->SetData(TEXT("identityMatrix"), &IDENTITYMATRIX);

    float DeltaTime = GetWorld()->getDeltaTime();
    TickBuffer->SetData(TEXT("DeltaTime"), &DeltaTime);
    float TotalTime = GetWorld()->GetTotalTime();
    TickBuffer->SetData(TEXT("Time"), &TotalTime);

    getGraphicDevice()->SetTickConstantBuffer(TickBuffer);
}

void MScene::UpdateBuffersFromMesh(uint32 InPID, std::shared_ptr<MMesh>& InMesh)
{
    uint32 PrimitiveDataNum = GetSize(PrimitiveDatas[InPID]);

    if (PrimitiveDataNum == 0)
    {
        // PrimitiveData의 Buffer를 채우는 함수인데, PrimitiveData가 없으면 안됨
        return;
    }

    FMeshBufferContainer SharedBuffers = {};
    getGraphicDevice()->GetBuffers(SharedBuffers, InMesh);

    FMeshBufferContainer PrivateBuffers = {};
    getGraphicDevice()->GetPrivateBuffers(PrivateBuffers, InPID);

    for (uint32 i = 0; i < PrimitiveDataNum; ++i)
    {
        FPrimitiveData& PrimitiveData = PrimitiveDatas[InPID][i];

        auto& Iter = PrivateBuffers.VertexBuffers.find(i);
        PrimitiveData.VertexBuffer = Iter == PrivateBuffers.VertexBuffers.end() ? SharedBuffers.VertexBuffers[i] : Iter->second;

        auto& Iter2 = PrivateBuffers.IndexBuffers.find(i);
        PrimitiveData.IndexBuffer = Iter2 == PrivateBuffers.IndexBuffers.end() ? SharedBuffers.IndexBuffers[i] : Iter2->second;
    }
}

void MScene::UpdateBuffersFromShader(uint32 InPID, std::shared_ptr<MShader> InShader)
{
    //uint32 PrimitiveDataNum = GetSize(PrimitiveDatas[InPID]);

    //if (auto FXComp = PrimitiveDatas[InPID][0].PrimitiveComponent.lock()->CastToShared<MFXComponent>())
    //{
    //    UINT StructSize = static_cast<uint32>(sizeof(FParticle));
    //    UINT ElementNum = static_cast<UINT>(GetSize(FXComp->Particles));

    //    if (InShader->bUseStructuredBuffer)
    //    {

    //    }

    //    if (InShader->bUseRWStructuredBuffer)
    //    {

    //    }

    //    for (uint32 i = 0; i < PrimitiveDataNum; ++i)
    //    {
    //        FPrimitiveData& PrimitiveData = PrimitiveDatas[InPID][i];

    //        if (PrimitiveData.StructuredBuffer.GetBufferID() == 0)
    //        {
    //            PrimitiveData.StructuredBuffer = getGraphicDevice()->AddStructuredBuffer(FXComp->Particles.data(), StructSize * ElementNum, ElementNum, StructSize, true);
    //        }
    //        else
    //        {
    //            getGraphicDevice()->UpdateStructuredBuffer(PrimitiveData.StructuredBuffer, FXComp->Particles.data(), StructSize * ElementNum, ElementNum, StructSize);
    //        }
    //    }
    //}
}

void MScene::UpdatePrimitiveData(MPrimitiveComponent* InComponent)
{
    uint32 PrimitiveID = InComponent->GetPrimitiveID();
    ClearPrimtiveDatas(PrimitiveID);

    std::shared_ptr<MMesh> Mesh = nullptr;
    if (auto& MeshComp = InComponent->CastToShared<MMeshComponent>())
    {
        Mesh = MeshComp->GetMesh();
    }

    if (Mesh == nullptr)
    {
        return;
    }

    // Component로부터 PrimitiveData 생성
    std::vector<FPrimitiveData> NewPrimitiveDatas;
    if (InComponent->GetPrimitiveData(NewPrimitiveDatas))
    {
        AddPrimitiveDatas(PrimitiveID, NewPrimitiveDatas);
    }
}

void MScene::AddPrimitiveDatas(uint32 InPID, const std::vector<FPrimitiveData>& InPrimitiveDatas)
{
    PrimitiveDatas[InPID].insert(PrimitiveDatas[InPID].end(), InPrimitiveDatas.begin(), InPrimitiveDatas.end());
}

void MScene::ClearPrimtiveDatas(uint32 InPID)
{
    PrimitiveDatas[InPID].clear();
}

const std::vector<FPrimitiveData>& MScene::GetPrimitiveDatas(uint32 InPrimitiveID)
{
    auto& Iter = PrimitiveDatas.find(InPrimitiveID);
    if (Iter != PrimitiveDatas.end())
    {
        return PrimitiveDatas[InPrimitiveID];
    }

    return PrimitiveDatas[-1];
}

const std::map<uint32, std::vector<FPrimitiveData>>& MScene::GetPrimitiveDatas() const
{
    return PrimitiveDatas;
}

void MScene::AddRenderablePrimitiveDatas(const std::vector<FPrimitiveData>& InPrimitiveDatas)
{
    RenderablePrimitiveData.insert(RenderablePrimitiveData.end(), InPrimitiveDatas.begin(), InPrimitiveDatas.end());
}

void MScene::AddRenderablePrimitiveData(const FPrimitiveData& InPrimitiveData)
{
    RenderablePrimitiveData.push_back(InPrimitiveData);
}

std::vector<FPrimitiveData>& MScene::GetRenderablePrimitiveData()
{
    return RenderablePrimitiveData;
}

void MScene::AddTemporalPrimitiveData(const FPrimitiveData& InPrimitiveData)
{
    TemporalPrimitiveDatas.push_back(InPrimitiveData);
}

const std::vector<FPrimitiveData>& MScene::GetTemporalPrimitiveDatas() const
{
    return CachedTemporalPrimitiveDatas;
}

void MScene::AddInstanceData(MObject* InDataOwner, uint32 InSize)
{
    InstacingDatas[InDataOwner].resize(static_cast<size_t>(InSize));
}
 
void MScene::AddTemporalInstanceData(const std::wstring& InKey, FPrimitiveData& InPrimitiveData, const std::shared_ptr<MVertexBuffer> InInstanceBuffer)
{
    if (TemporalInstanceBuffers.find(InKey) == TemporalInstanceBuffers.end())
    {
        TemporalInstanceBuffers[InKey] = InInstanceBuffer;
    }

    // 인스턴싱을 사용하더라도, PrimitiveData가 하나는 추가되야 함
    if (TemporalInstanceDatas.find(InKey) == TemporalInstanceDatas.end())
    {
        InPrimitiveData.InstanceBuffer = InInstanceBuffer;
        AddTemporalPrimitiveData(InPrimitiveData);
    }

    FVertex_Instance NewInstance = {};
    TransformMatrix(NewInstance.WorldMatrix, InPrimitiveData.Scale, InPrimitiveData.Rotation, InPrimitiveData.Translation);
    XMStoreFloat4x4(&NewInstance.WorldMatrix, XMLoadFloat4x4(&NewInstance.WorldMatrix) * XMLoadFloat4x4(&GetWorld()->getMainCameraViewMatrix()) * XMLoadFloat4x4(&GetWorld()->getMainCameraProjectioinMatrix()));
    TemporalInstanceDatas[InKey].push_back(NewInstance);
}
