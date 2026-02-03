#include "SSAOPass.h"
#include "MoonEngine.h"
#include "Renderer.h"

//#include "CombinePass.h"

#include "Camera.h"

//void MSSAOPass::UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData)
//{
//    auto Camera = getRenderer()->GetWorld()->getMainCamera();
//
//    if (std::shared_ptr<MShader>& PixelShader = GetPixelShader(PrimitiveData))
//    {
//        PixelShader->SetValue(TEXT("InvProjMatrix"), Camera->getInversePerspectiveProjectionMatrix());
//    }
//
//    MFullScreenQuadPass::UpdateRenderPassConstantBuffer(PrimitiveData);
//}
