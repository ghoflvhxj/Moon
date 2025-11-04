#include "EditorPass.h"

MEditorPass::MEditorPass()
    : MRenderPass()
{
    SetDepthEnable(false);
    SetDefaultShader(TEXT("VS_Collision.cso"), TEXT("PS_Collision.cso"));
    DefaultTopology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
}

bool MEditorPass::IsValidPrimitive(const FPrimitiveData& PrimitiveData) const
{
    if (MRenderPass::IsValidPrimitive(PrimitiveData))
    {
        return PrimitiveData.PrimitiveType == EPrimitiveType::CustomPrimitiveType0;
    }

    return false;
}