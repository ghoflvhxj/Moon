#pragma once
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

#include "Render.h"
#include "PrimitiveComponent.h"

class ENGINE_DLL RenderPass abstract
{
public:
	explicit RenderPass();
	virtual ~RenderPass();

public:
	void begin();
	void end();

	virtual void doPass(RenderQueue &renderQueue) = 0;

public:
	void initializeRenderTarget(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList);
private:
	std::vector<std::shared_ptr<RenderTarget>> _renderTargetList;

public:
	void initializeResourceViewByRenderTarget(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList);
private:
	std::vector<std::shared_ptr<TextureComponent>> _resourceViewList;

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
	std::shared_ptr<VertexShader>	_vertexShader;
	std::shared_ptr<PixelShader>   _pixelShader;
	bool _bShaderSet;
	bool _bRenderTargetSet;
	uint32 _renderTargetCount;
};

#endif