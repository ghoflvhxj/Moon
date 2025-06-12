#pragma once
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include "Render.h"
#include "PrimitiveComponent.h"

struct FViewBindData
{
	FViewBindData() 
		: Index(-1)
	{}
	int32 Index;
	std::shared_ptr<RenderTarget> ReourceView;
};

class ENGINE_DLL RenderPass abstract
{
public:
	static const int RT_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

public:
	explicit RenderPass();
	virtual ~RenderPass();

public:
	virtual void Begin();
	/*virtual*/ void End();
	virtual void Render(const std::vector<FPrimitiveData>& PrimitiveDatList);
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const;
	virtual void UpdateObjectConstantBuffer(const FPrimitiveData& PrimitiveData);
    virtual void DrawPrimitive(const FPrimitiveData& PrimitiveData);

protected:
    virtual void HandleInputAssemblerStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleVertexShaderStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleGeometryShaderStage(const FPrimitiveData& PrimitiveData);
    virtual void HandlePixelShaderStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleRasterizerStage(const FPrimitiveData& PrimitiveData);
    virtual void HandleOuputMergeStage(const FPrimitiveData& PrimitiveData);
protected:
    bool bWriteDepthStencil = true;


public:
	// 렌더 타겟 바인드
	template<class... TList>
	void BindRenderTargets(RenderTargets& renderTargetList, TList... args)
	{
		bRenderTarget = true;
		CachedRenderTargets = renderTargetList;
		BindView(renderTargetList, RenderTargetViewData, args...);
	}
	// 쉐이더 리소스 뷰 바인드
	template<class... TList>
	void BindResourceViews(RenderTargets& renderTargetList, TList... args)
	{
		CachedResourceViews = renderTargetList;
		BindView(renderTargetList, ResourceViewData, args...);
	}
protected:
	template<class T, class... TList>
	void BindView(RenderTargets& Source, std::vector<FViewBindData>& Target, T arg)
	{
		int32 Index = CastValue<int32>(arg);
		FViewBindData ResourceViewBindData;
		ResourceViewBindData.Index = Index;
		ResourceViewBindData.ReourceView = Source[Index];

		Target.push_back(ResourceViewBindData);
	}
	template<class T, class... TList>
	void BindView(RenderTargets& Source, std::vector<FViewBindData>& Target, T arg, TList... args)
	{
		BindView(Source, Target, arg);
		BindView(Source, Target, args...);
	}

protected:
	std::vector<FViewBindData> RenderTargetViewData;
	std::vector<FViewBindData> ResourceViewData;
	bool bRenderTarget = false;
	RenderTargets CachedRenderTargets;
	RenderTargets CachedResourceViews;


private:
	ID3D11RenderTargetView *_pOldRenderTargetView;
	ID3D11DepthStencilView *_pOldDepthStencilView;
	
public:
	void setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName);
	void setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName, const wchar_t *geomtryShaderFileName);
	const bool isShaderSet() const;
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

public:
	void SetClearTargets(const bool bClear);
private:
	bool bClearTargets;

public:
	void SetUseOwningDepthStencilBuffer(const ERenderTarget bUse);
private:
	ERenderTarget UsedDepthStencilBuffer;

public:
	DirectX::XMVECTORF32 Color = EngineColors::Black;
};

#endif