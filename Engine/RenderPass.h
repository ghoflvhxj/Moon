#pragma once

#include "Include.h"

#include "Render.h"
#include "PrimitiveComponent.h"

class MMaterial;
class MComputeShader;

struct FRenderTargetBindData
{
	FRenderTargetBindData() 
        :Index(ERenderTarget::Count)
	{
    }
    ERenderTarget Index;
	//int32 Index;
	//std::shared_ptr<MRenderTarget> RenderTarget;
};

class ENGINE_DLL MRenderPass
{
public:
	static const int RT_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

public:
	explicit MRenderPass();
	virtual ~MRenderPass();

public:
    // 매 프레임마다 렌더 패스가 시작될 때 한번 호출됨. 한번만 설정해야 된다면 여기서 작업하는 것이 좋음.
	virtual void Begin();
    // 매 프레임마다 렌더 패스가 종료될 때 한번 호출됨
	virtual void End();
    virtual void RenderPass(std::vector<FPrimitiveData>& PrimitiveDatList);

    void Clear();

protected:
    virtual void DrawPrimitive(const FPrimitiveData& PrimitiveData);
    virtual bool IsValidPrimitive(const FPrimitiveData& InPrimitiveData) const;

protected:
    virtual void UpdateRenderPassConstantBuffer(std::shared_ptr<MShader> InShader);
    virtual void UpdateRenderPassObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& PrimitiveData);
	virtual void UpdateObjectConstantBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& PrimitiveData);
    virtual void UpdateMaterialConstantBuffer(std::shared_ptr<MShader> InShader, std::shared_ptr<MMaterial>& InMaterial, const FPrimitiveData& PrimitiveData);
    virtual void UpdateStructuredBuffer(std::shared_ptr<MShader> InShader, const FPrimitiveData& InPrimitiveData);

protected:
    virtual void HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleVertexShaderStage(std::shared_ptr<MVertexShader> InVertexShader, const FPrimitiveData& PrimitiveData);
    virtual void HandleGeometryShaderStage(std::shared_ptr<MGeometryShader> InGeometryShader, const FPrimitiveData& PrimitiveData);
    virtual void HandlePixelShaderStage(std::shared_ptr<MPixelShader> InPixelShader, const FPrimitiveData& PrimitiveData);
    virtual void HandleComputeShaderStage(MComputeShader* InComputeShader , FPrimitiveData& InPrimitiveData);
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData);
protected:
    bool bWriteDepthStencil = true;

public:
    FDelegate<void, const FPrimitiveData&, std::shared_ptr<MShader>>& GetHandlePixelShaderStageDelegate() { return OnHandlePxielShaderStage; }
protected:
    FDelegate<void, const FPrimitiveData&, std::shared_ptr<MShader>> OnHandlePxielShaderStage;

protected:
    std::shared_ptr<MVertexShader> GetVertexShader(const FPrimitiveData& InPrimitiveData);
    std::shared_ptr<MPixelShader> GetPixelShader(const FPrimitiveData& InPrimitiveData);
    std::shared_ptr<MGeometryShader> GetGeometryShader(const FPrimitiveData& InPrimitiveData);

public:
	// 렌더 타겟 바인드
	template<class... TList>
	void BindRenderTargets(RenderTargets& InRenderTargets, TList... args)
	{
		BindView(InRenderTargets, RenderTargetViewData, args...);
	}
	// 쉐이더 리소스 뷰 바인드
	template<class... TList>
	void BindResourceViews(RenderTargets& InRenderTargets, TList... args)
	{
		BindView(InRenderTargets, ResourceViewData, args...);
	}
protected:
	template<class T, class... TList>
	void BindView(RenderTargets& Source, std::vector<FRenderTargetBindData>& Target, T arg)
	{
		FRenderTargetBindData ResourceViewBindData;
		ResourceViewBindData.Index = arg;

		Target.push_back(ResourceViewBindData);
	}
	template<class T, class... TList>
	void BindView(RenderTargets& Source, std::vector<FRenderTargetBindData>& Target, T arg, TList... args)
	{
		BindView(Source, Target, arg);
		BindView(Source, Target, args...);
	}

protected:
	std::vector<FRenderTargetBindData> RenderTargetViewData;
	std::vector<FRenderTargetBindData> ResourceViewData;

public:
    // 렌더 타겟 리소스 뷰들을 매터리얼 취급할지
    bool bLikeMaterial = false;

public:
    void SetDefaultShader(EShaderType InShaderType, const std::wstring& InFileName);
	void SetDefaultShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName);
	void SetDefaultShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName, const wchar_t *geomtryShaderFileName);
	const bool isShaderSet() const;
    void ApplyDefaultShaderOnly(bool InValue) { bUseDefaultShaderOnly = InValue; }
private:
	void releaseShader();
protected:
	std::wstring	_vertexShaderFileName;
	std::wstring	_pixelShaderFileName;
	std::wstring	_geometryShaderFileName;
	std::shared_ptr<MVertexShader>	_vertexShader;
	std::shared_ptr<MPixelShader>	_pixelShader;
	std::shared_ptr<MGeometryShader> GeometryShader;
    MComputeShader ComputeShader;
	bool _bShaderSet;
    bool bUseDefaultShaderOnly = false;

public:
	void SetClearTargets(const bool bClear);
private:
	bool bClearTargets = true;

public:
    void SetDepthEnable(const bool InEnable) { bDepthEnable = InEnable; }
protected:
    bool bDepthEnable = true;

    // 렌더 타겟이 뎁스 전용인 경우만 자기것을 사용하니 필요없으나, 일단은 남겨둠
//public:
//	void SetUseOwningDepthStencilBuffer(const ERenderTarget bUse);
//private:
//	ERenderTarget UseOwningDepthStencilBuffer;

public:
    void UseCommonDepthStencil() { bUseCommonDepthStencil = true; }
    bool bUseCommonDepthStencil = false;

protected:
    D3D_PRIMITIVE_TOPOLOGY DefaultTopology = D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	DirectX::XMVECTORF32 Color = EngineColors::Black;
};
;