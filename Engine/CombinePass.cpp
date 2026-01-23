#include "CombinePass.h"
#include "MoonEngine.h"

#include "MainGameSetting.h"

// Renderer
#include "Renderer.h"

// Graphic
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "RenderTarget.h"

// Game
#include "World.h"

// Actor
#include "Camera.h"

// 임시
#include "LightComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"

#include "VertexBuffer.h"


#undef max
#undef min

using namespace DirectX;

GeometryPass::GeometryPass()
    : MRenderPass()
{
    UseCommonDepthStencil();
}

bool GeometryPass::IsValidPrimitive(const FPrimitiveData &PrimitiveData) const
{
	return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && MRenderPass::IsValidPrimitive(PrimitiveData);
}

DirectionalShadowDepthPass::DirectionalShadowDepthPass()
	: MRenderPass()
{
    bUseDefaultShaderOnly = true;
    ViewProj.resize(CastValue<size_t>(ECascade::Count));
    Transforms.resize(CastValue<size_t>(ECascade::Count));

    std::vector<FVertex_Instance> InstancingData(4);
    uint32 Size = static_cast<uint32>(sizeof(FVertex_Instance));
    uint32 Num = GetSize(InstancingData);
    InstanceBuffer = std::make_shared<MVertexBuffer>(Size, Num, InstancingData.data(), true);
}

void DirectionalShadowDepthPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    auto& LightPrimitiveDatas = getRenderer()->GetPrimitiveDatas(EPrimitiveType::DirectionalLight);
    for(uint32 LightIndex = 0; LightIndex < GetSize(LightPrimitiveDatas); ++LightIndex)
    {
        const std::shared_ptr<MLightComponent>& LightComponent = LightPrimitiveDatas[LightIndex]->GetPrimitiveComponent<MLightComponent>();
        assert(LightComponent);

        MWorld* World = LightComponent->GetWorld();
        assert(World);

        const FWorldRenderInfo& Info = GetEngine()->GetWorldInfo(World->GetID());
        auto& Window = Info.DstWindow;
        float AspectRatio = Window->GetAspectRatio();
        if (std::isnan(AspectRatio))
        {
            return;
        }

        auto& Camera = World->getMainCamera();
        float tanHalfVertical = tanf(XMConvertToRadians(Camera->getFov() / 2.f));
        float tanHalfHorizen = tanHalfVertical * AspectRatio;

        XMMATRIX XMCameraWorldMat = XMLoadFloat4x4(&Camera->getInvesrViewMatrix());
        XMVECTOR XMLightDirection = XMVector3Normalize(XMLoadFloat3(&LightComponent->GetDirection()));
        XMVECTOR XMUpVector = XMLoadFloat3(&VEC3UP);
        if (fabs(XMVectorGetX(XMVector3Dot(XMUpVector, XMLightDirection))) > 0.999f)
        {
            XMUpVector = XMVectorSet(1.f, 0, 0, 0);
        }

        MScene* Scene = getRenderer()->GetCurrentScene();
        assert(Scene);
        for (int CascadeIndex = 0; CascadeIndex < CastValue<int>(ECascade::Far); ++CascadeIndex)
        {
            float CascadeNear = Scene->GetCascadeDistance(CascadeIndex);
            float CascadeFar = Scene->GetCascadeDistance(CascadeIndex + 1);

            float XNear = CascadeNear * tanHalfHorizen;
            float XFar = CascadeFar * tanHalfHorizen;
            float YNear = CascadeNear * tanHalfVertical;
            float YFar = CascadeFar * tanHalfVertical;
            float DepthCenter = (CascadeNear + CascadeFar) / 2.f;

            std::vector<Vec3> FrustumVertices = {
                //near Face
                {XNear,YNear,CascadeNear},
                {-XNear,YNear,CascadeNear},
                {XNear,-YNear,CascadeNear},
                {-XNear,-YNear,CascadeNear},
                //far Face
                {XFar,YFar,CascadeFar},
                {-XFar,YFar,CascadeFar},
                {XFar,-YFar,CascadeFar},
                {-XFar,-YFar,CascadeFar}
            };

            XMVECTOR XMCenterPosInWorld = {};
            for (auto& FrustumVertex : FrustumVertices)
            {
                XMStoreFloat3(&FrustumVertex, XMVector3TransformCoord(XMLoadFloat3(&FrustumVertex), XMCameraWorldMat));
                XMCenterPosInWorld += XMLoadFloat3(&FrustumVertex);
            }
            XMCenterPosInWorld /= static_cast<float>(FrustumVertices.size());

            float Radius = 0.f;
            for (auto& FrustumVertex : FrustumVertices)
            {
                float Length = XMVectorGetX(XMVector3Length(XMLoadFloat3(&FrustumVertex) - XMCenterPosInWorld));
                Radius = std::max<float>(Length, Radius);
            }
            Radius = std::ceil(Radius * 2.f) / 2.f;

            float Temp = Radius;
            Temp = std::max(Radius, 100.f);
            XMVECTOR Eye = XMCenterPosInWorld - (XMLightDirection * Temp);
            XMVECTOR Focus = XMCenterPosInWorld;
            XMMATRIX LightView = XMMatrixLookAtLH(Eye, Focus, XMUpVector);

            float Near = GraphicDevice::bReverseDepth ? Temp * 2.f : 0.1f;
            float Far = GraphicDevice::bReverseDepth ? 0.1f : Temp * 2.f;

            XMMATRIX OrthoProjMatrix = XMMatrixOrthographicOffCenterLH(-Radius, Radius, -Radius, Radius, Near, Far);

            XMStoreFloat4x4(&ViewProj[CascadeIndex], LightView * OrthoProjMatrix);
        }

        MRenderPass::RenderPass(PrimitiveDatList);
    }
}

void DirectionalShadowDepthPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    getGraphicDevice()->DrawInstance(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock(), InstanceBuffer);
}

bool DirectionalShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        if (PrimitiveData.PrimitiveType == EPrimitiveType::Mesh)
        {
            if (PrimitiveData.PrimitiveComponent.lock())
            {
                return PrimitiveData.PrimitiveComponent.lock()->IsShadowing();
            }
        }
    }

    return false;
}

void DirectionalShadowDepthPass::UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    if (auto& PrimitiveComp = PrimitiveData.GetPrimitiveComponent<MPrimitiveComponent>())
    {
        for (uint32 i = 0; i < GetSize(Transforms); ++i)
        {
            XMMATRIX XMWorldMat = XMLoadFloat4x4(&PrimitiveComp->getWorldMatrix());
            XMStoreFloat4x4(&Transforms[i], XMWorldMat * XMLoadFloat4x4(&ViewProj[i]));
        }
    }

    // 이거는 윗단에서 자동으로 되도록 구현해줘야 함
    auto& VS = GetVertexShader(PrimitiveData);
    if (VS)
    {
        VS->SetValue(TEXT("Transforms"), Transforms);
    }
}

const std::vector<Mat4>& DirectionalShadowDepthPass::GetViewProjs() const
{
    return ViewProj;
}

PointShadowDepthPass::PointShadowDepthPass()
    : MRenderPass()
{
    bUseDefaultShaderOnly = true;
    Color = { 1000.f, 1000.f, 1000.f, 1.f };
    ViewProj.resize(6);
    Transforms.resize(6);

    std::vector<FVertex_Instance> InstancingData(6);
    uint32 Size = static_cast<uint32>(sizeof(FVertex_Instance));
    uint32 Num = GetSize(InstancingData);
    InstanceBuffer = std::make_shared<MVertexBuffer>(Size, Num, InstancingData.data(), true);
}

void PointShadowDepthPass::RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList)
{
    auto& PointLightPrimitives = g_pRenderer->GetPrimitiveDatas(EPrimitiveType::PointLight);
    uint32 PointLightNum = GetSize(PointLightPrimitives);

    for (uint32 i = 0; i < PointLightNum; ++i)
    {
        PointLightIndex = i;

        const FPrimitiveData& PrimitiveData = *PointLightPrimitives[i];
        std::shared_ptr<MPointLightComponent>& LightComponent = PrimitiveData.GetPrimitiveComponent<MPointLightComponent>();

        if (LightComponent->IsShadowing() == false)
        {
            continue;
        }

        // 콘스탄트 버퍼 업데이트
        LightPos = LightComponent->getTranslation();

        XMVECTOR XMPosition = XMLoadFloat3(&LightPos);
        XMVECTOR Up = XMLoadFloat3(&VEC3UP);

        // 순서는 오른쪽, 왼쪽, 위, 아래, 앞, 뒤
        /*
        std::vector<Mat4> ViewMat(6);
        XMStoreFloat4x4(&ViewMat[0], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(1.f, 0.f, 0.f, 0.f), Up));
        XMStoreFloat4x4(&ViewMat[1], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(-1.f, 0.f, 0.f, 0.f), Up));
        XMStoreFloat4x4(&ViewMat[2], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 0.f)));
        XMStoreFloat4x4(&ViewMat[3], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, -1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 0.f)));
        XMStoreFloat4x4(&ViewMat[4], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 0.f, 1.f, 0.f), Up));
        XMStoreFloat4x4(&ViewMat[5], XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 0.f, -1.f, 0.f), Up));
        
        float Near = GraphicDevice::bReverseDepth ? 1000.f : 0.1f;
        float Far = GraphicDevice::bReverseDepth ? 0.1f : 1000.f;
        XMMATRIX XMProjMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, Near, Far);
        for (uint32 i = 0; i < 6; ++i)
        {
            XMStoreFloat4x4(&ViewProj[i], XMLoadFloat4x4(&ViewMat[i]) * XMProjMat);
        }
        */

        std::vector<XMMATRIX> XMViewMats(6);
        XMViewMats[0] = XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(1.f, 0.f, 0.f, 0.f), Up);
        XMViewMats[1] = XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(-1.f, 0.f, 0.f, 0.f), Up);
        XMViewMats[2] = XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 0.f));
        XMViewMats[3] = XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, -1.f, 0.f, 0.f), XMVectorSet(0.f, 0.f, 1.f, 0.f));
        XMViewMats[4] = XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 0.f, 1.f, 0.f), Up);
        XMViewMats[5] = XMMatrixLookAtLH(XMPosition, XMPosition + XMVectorSet(0.f, 0.f, -1.f, 0.f), Up);

        float Near = GraphicDevice::bReverseDepth ? 1000.f : 0.1f;
        float Far = GraphicDevice::bReverseDepth ? 0.1f : 1000.f;
        XMMATRIX XMProjMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, Near, Far);
        for (uint32 i = 0; i < 6; ++i)
        {
            XMStoreFloat4x4(&ViewProj[i], XMViewMats[i] * XMProjMat);
        }

        MRenderPass::RenderPass(PrimitiveDatList);
    }
}

void PointShadowDepthPass::DrawPrimitive(const FPrimitiveData& PrimitiveData)
{
    getGraphicDevice()->DrawInstance(PrimitiveData.VertexBuffer.lock(), PrimitiveData.IndexBuffer.lock(), InstanceBuffer);
}

bool PointShadowDepthPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && PrimitiveData.PrimitiveComponent.lock()->IsShadowing();
    }

    return false;
}

void PointShadowDepthPass::UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData)
{
    if (auto& PrimitiveComp = PrimitiveData.GetPrimitiveComponent<MPrimitiveComponent>())
    {
        XMMATRIX XMWorldMat = XMLoadFloat4x4(&PrimitiveComp->getWorldMatrix());
        for (uint32 i = 0; i < GetSize(Transforms); ++i)
        {
            XMStoreFloat4x4(&Transforms[i], XMWorldMat * XMLoadFloat4x4(&ViewProj[i]));
        }
    }

    auto VS = GetVertexShader(PrimitiveData);
    if (VS)
    {
        VS->SetValue(TEXT("LightPos"), LightPos);
        VS->SetValue(TEXT("PointLightIndex"), PointLightIndex);
        VS->SetValue(TEXT("Transforms"), Transforms);
    }
}

void PointShadowDepthPass::HandleOutputMergeStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->LinearDepthStencil();
}

bool SkyPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
	if (PrimitiveData.PrimitiveType != EPrimitiveType::Sky)
	{
		return false;
	}

	return MRenderPass::IsValidPrimitive(PrimitiveData);
}

MLinePass::MLinePass()
    : MRenderPass()
{
    DefaultTopology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
}

bool MLinePass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    return g_pRenderer->GetCurrentScene()->IsDrawCollision() && PrimitiveData.PrimitiveType == EPrimitiveType::Collision && MRenderPass::IsValidPrimitive(PrimitiveData);
}

MDepthPre::MDepthPre()
{
    UseCommonDepthStencil();
}

bool MDepthPre::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        bool bAlphaMasked = false;
        if (std::shared_ptr<MMaterial> Material = PrimitiveData.Material.lock())
        {
            bAlphaMasked = Material->IsAlphaMasked();
        }

        return PrimitiveData.PrimitiveType == EPrimitiveType::Mesh && bAlphaMasked == false;
    }

    return false;
}

void MDepthPre::HandleRasterizerStage(const FPrimitiveData& PrimitiveData)
{
    g_pGraphicDevice->RSDepthPre();
}
