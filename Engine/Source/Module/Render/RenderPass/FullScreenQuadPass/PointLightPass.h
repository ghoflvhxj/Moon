#include "FullScreenQuadPass.h"

class MPrimitiveComponent;

class PointLightPass : public MFullScreenQuadPass
{
public:
    explicit PointLightPass();
    virtual ~PointLightPass() = default;

public:
    virtual void End() override;
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    //virtual void UpdateRenderPassConstantBuffer(std::shared_ptr<MShader> InShader) override;
    virtual void UpdateRenderPassObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOutputMergeStage(const FPrimitiveData& primitiveData) override;

public:
    virtual std::vector<FPrimitiveData> MakePrimitiveDatas() override;

protected:
    uint32 Indexer = 0;

    Vec4 LightPosAndRange;
    Vec4 LightColorAndIntensity;

};
