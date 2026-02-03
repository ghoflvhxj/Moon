#include "FullScreenQuadPass.h"

class DirectionalLightPass : public MFullScreenQuadPass
{
public:
    explicit DirectionalLightPass();
    virtual ~DirectionalLightPass() = default;

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
    virtual void UpdateRenderPassObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& InPrimitiveData) override;

public:
    virtual std::vector<FPrimitiveData> MakePrimitiveDatas() override;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOutputMergeStage(const FPrimitiveData& primitiveData) override;

protected:
    Mat4 InverseProj = IDENTITYMATRIX;
    Mat4 InverseCameraView = IDENTITYMATRIX;
    Vec3 Direction = VEC3ZERO;
    Vec4 ColorAndIntensity = VEC4ONE;
    Vec3 Ambient = VEC3ONE;
};
