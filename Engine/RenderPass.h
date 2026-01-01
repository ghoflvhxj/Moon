#pragma once

#include "Include.h"

#include "Render.h"
#include "PrimitiveComponent.h"

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
    virtual void RenderPass(const std::vector<FPrimitiveData>& PrimitiveDatList);

protected:
    // 매 프레임마다 렌더 패스가 시작될 때 한번 호출됨. 한번만 설정해야 된다면 여기서 작업하는 것이 좋음.
	virtual void Begin();
    // 매 프레임마다 렌더 패스가 종료될 때 한번 호출됨
	virtual void End();
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const;
    virtual void UpdateTickConstantBuffer(const FPrimitiveData& PrimitiveData);
	virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData);
    virtual void DrawPrimitive(const FPrimitiveData& PrimitiveData);

protected:
    virtual void HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleVertexShaderStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleGeometryShaderStage(const FPrimitiveData& PrimitiveData);
    virtual void HandlePixelShaderStage(const FPrimitiveData& PrimitiveData);

protected:
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData);
protected:
    uint32 RectWidth = 0;
    uint32 RectHeight = 0;

    virtual void HandleOutputMergeStage(const FPrimitiveData& PrimitiveData);
protected:
    bool bWriteDepthStencil = true;

protected:
    std::shared_ptr<MShader> GetVertexShader(const FPrimitiveData& InPrimitiveData);
    std::shared_ptr<MShader> GetPixelShader(const FPrimitiveData& InPrimitiveData);

public:
	// 렌더 타겟 바인드
	template<class... TList>
	void BindRenderTargets(RenderTargets& InRenderTargets, TList... args)
	{
		bRenderTarget = true;
		//CachedRenderTargets = renderTargetList;
		BindView(InRenderTargets, RenderTargetViewData, args...);
	}
	// 쉐이더 리소스 뷰 바인드
	template<class... TList>
	void BindResourceViews(RenderTargets& InRenderTargets, TList... args)
	{
		//CachedResourceViews = InRenderTargets;
		BindView(InRenderTargets, ResourceViewData, args...);
	}
protected:
	template<class T, class... TList>
	void BindView(RenderTargets& Source, std::vector<FRenderTargetBindData>& Target, T arg)
	{
		//int32 Index = CastValue<int32>(arg);
		FRenderTargetBindData ResourceViewBindData;
		ResourceViewBindData.Index = arg;
		//ResourceViewBindData.RenderTarget = Source[Index];

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
	bool bRenderTarget = false;
	//RenderTargets CachedRenderTargets;
	//RenderTargets CachedResourceViews;

public:
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
	std::shared_ptr<MShader>	_vertexShader;
	std::shared_ptr<MShader>	_pixelShader;
	std::shared_ptr<MShader> _geometryShader;
	bool _bShaderSet;
    bool bUseDefaultShaderOnly = false;

public:
	void SetClearTargets(const bool bClear);
private:
	bool bClearTargets;

public:
    void SetDepthEnable(const bool InEnable) { bDepthEnable = InEnable; }
protected:
    bool bDepthEnable = true;

public:
	void SetUseOwningDepthStencilBuffer(const ERenderTarget bUse);
private:
	ERenderTarget UseOwningDepthStencilBuffer;
public:
    void UseCommonDepthStencil() { bUseCommonDepthStencil = true; }
    bool bUseCommonDepthStencil = false;

protected:
    D3D_PRIMITIVE_TOPOLOGY DefaultTopology = D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	DirectX::XMVECTORF32 Color = EngineColors::Black;
};
