#pragma once
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include "Render.h"
#include "PrimitiveComponent.h"

class ENGINE_DLL RenderPass abstract
{
public:
	static const int RT_COUNT = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;

public:
	explicit RenderPass();
	virtual ~RenderPass();

public:
	virtual void begin();
	/*virtual*/ void end();
	virtual void doPass(RenderQueue &renderQueue);
	virtual const bool processPrimitiveData(PrimitiveData &primitiveData) = 0;
	virtual void render(PrimitiveData &primitiveData) = 0;

public:
	template<class... TList>
	void initializeRenderTargets(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList, TList... args)
	{
		bindFunction = [&](std::vector<std::shared_ptr<RenderTarget>> &bindList, int index)->void {
			bindList[index] = renderTargetList[index];
		};

		makeBindList(renderTargetList, _renderTargetList, args...);

		_bRenderTargetSet = true;
		_renderTargetCount = CastValue<uint32>(_renderTargetList.size());
	}
private:
	std::vector<std::shared_ptr<RenderTarget>> _renderTargetList;

public:
	template<class... TList>
	void initializeResourceViews(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList, TList... args)
	{
		bindFunction = [&](std::vector<std::shared_ptr<RenderTarget>> &bindList, int index)->void {
			bindList[index] = renderTargetList[index];
		};

		makeBindList(renderTargetList, _resourceViewList, args...);
	}
private:
	std::vector<std::shared_ptr<RenderTarget>> _resourceViewList;

private:
	template<class T, class... TList>
	void makeBindList(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList, std::vector<std::shared_ptr<RenderTarget>> &bindList, T arg)
	{
		bindFunction(bindList, (int)arg);
	}

	template<class T, class... TList>
	void makeBindList(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList, std::vector<std::shared_ptr<RenderTarget>> &bindList, T arg, TList... args)
	{
		bindFunction(bindList, (int)arg);
		makeBindList(renderTargetList, bindList, args...);
	}

	std::function<void(std::vector<std::shared_ptr<RenderTarget>>&, int)> bindFunction;

private:
	ID3D11RenderTargetView *_pOldRenderTargetView;
	ID3D11DepthStencilView *_pOldDepthStencilView;
	
public:
	void setShader(const wchar_t *vertexShaderFileName, const wchar_t *pixelShaderFileName);
	const bool isShaderSet() const;
private:
	void releaseShader();
protected:
	std::wstring	_vertexShaderFileName;
	std::wstring	_pixelShaderFileName;
	std::shared_ptr<Shader>	_vertexShader;
	std::shared_ptr<Shader>	_pixelShader;
	bool _bShaderSet;
	bool _bRenderTargetSet;
	uint32 _renderTargetCount;

public:
	void SetClearTargets(const bool bClear);
private:
	bool _bClearTargets;
};

#endif