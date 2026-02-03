/*
    화면 전체를 덮는 사각형 메쉬를 그리는 패스.
    일반적으로 다른 패스의 결과물을 텍스쳐 리소스로 이용함.
*/

#pragma once 

#include "Include.h"

#include "Mesh/Mesh.h"

#include "Render.h"
#include "RenderPass.h"

class StaticMeshComponent;

class ENGINE_DLL MFullScreenQuadPass : public MRenderPass
{
public:
    MFullScreenQuadPass();
    virtual ~MFullScreenQuadPass() = default;

public:
    virtual void RenderPass(std::vector<FPrimitiveData>& PrimitiveDatList) override; 
    virtual std::vector<FPrimitiveData> MakePrimitiveDatas();

protected:
    FPrimitiveData CreatePrimitiveData(EPrimitiveType InType);

protected:
    FMeshData MeshData = {};
    uint32 PID = 0;
};

class ENGINE_DLL MCombinePass : public MFullScreenQuadPass
{
public:
    MCombinePass() = default;
    virtual ~MCombinePass() = default;

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData) override;
    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData) override;
};

class ENGINE_DLL MStencilPass : public MFullScreenQuadPass
{
public:
    MStencilPass() = default;
    virtual ~MStencilPass() = default;

protected:
    //virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const;

protected:
    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData) override;
};