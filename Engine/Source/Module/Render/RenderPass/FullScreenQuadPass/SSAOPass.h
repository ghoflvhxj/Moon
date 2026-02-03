#include "FullScreenQuadPass.h"

class ENGINE_DLL MSSAOPass : public MFullScreenQuadPass
{
public:
    MSSAOPass() = default;
    virtual ~MSSAOPass() = default;

public:
    //virtual void UpdateRenderPassConstantBuffer(const FPrimitiveData& PrimitiveData) override;
};