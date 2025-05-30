#pragma once
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include "Render.h"
#include "PrimitiveComponent.h"

struct FResourceViewBindData
{
	FResourceViewBindData() 
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
	virtual void DoPass(const std::vector<FPrimitiveData>& PrimitiveDatList);
	virtual const bool processPrimitiveData(const FPrimitiveData &primitiveData) = 0;
	virtual void render(const FPrimitiveData &primitiveData) = 0;

	// ·»´õ Å¸°Ù
public:
	template<class... TList>
	void initializeRenderTargets(RenderTargets& renderTargetList, TList... args)
	{
		bRenderTarget = true;
		CachedRenderTargets = renderTargetList;
		BindResourceView(renderTargetList, _renderTargetList, args...);
	}
	template<class... TList>
	void initializeResourceViews(RenderTargets& renderTargetList, TList... args)
	{
		CachedResourceViews = renderTargetList;
		BindResourceView(renderTargetList, _resourceViewList, args...);
	}
protected:
	std::vector<FResourceViewBindData> _renderTargetList;
	std::vector<FResourceViewBindData> _resourceViewList;
	bool bRenderTarget = false;
	RenderTargets CachedRenderTargets;
	RenderTargets CachedResourceViews;

	// À¯Æ¿
private:
	template<class T, class... TList>
	void BindResourceView(RenderTargets& Source, std::vector<FResourceViewBindData>& Target, T arg)
	{
		int32 Index = CastValue<int32>(arg);
		FResourceViewBindData ResourceViewBindData;
		ResourceViewBindData.Index			= Index;
		ResourceViewBindData.ReourceView	= Source[Index];

		Target.push_back(ResourceViewBindData);
	}
	template<class T, class... TList>
	void BindResourceView(RenderTargets& Source, std::vector<FResourceViewBindData>& Target, T arg, TList... args)
	{
		BindResourceView(Source, Target, arg);
		BindResourceView(Source, Target, args...);
	}

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
	std::shared_ptr<Shader>	_vertexShader;
	std::shared_ptr<Shader>	_pixelShader;
	std::shared_ptr<Shader> _geometryShader;
	bool _bShaderSet;

public:
	void SetClearTargets(const bool bClear);
private:
	bool bClearTargets;

public:
	void SetUseOwningDepthStencilBuffer(const bool bUse);
private:
	bool _bUseOwningDepthStencilBuffer;
};

#endif