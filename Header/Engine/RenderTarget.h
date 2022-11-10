#pragma once
#ifndef __RENDER_TARGET_H__

class TextureComponent;

struct RenderTagetInfo
{
	int textureArrayCount;
	UINT width;
	UINT height;
};

class RenderTarget
{
public:
	explicit RenderTarget();
	explicit RenderTarget(RenderTagetInfo &info);
	virtual ~RenderTarget();
	
public:
	std::shared_ptr<TextureComponent> AsTexture();
private:
	void initializeTexture(RenderTagetInfo &renderTargetInfo);
	std::shared_ptr<TextureComponent> _pRenderTargetTexture;
	std::shared_ptr<TextureComponent> _pDepthStencilTexture;

public:
	ID3D11RenderTargetView* AsRenderTargetView();
private:
	ID3D11RenderTargetView *_pRenderTargetView;

public:
	ID3D11DepthStencilView* getDepthStencilView();
private:
	ID3D11DepthStencilView *_pDepthStencilView;
};

#define __RENDER_TARGET_H__
#endif