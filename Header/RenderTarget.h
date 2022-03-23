#pragma once
#ifndef __RENDER_TARGET_H__

class MeshComponent;

class RenderTarget
{
public:
	explicit RenderTarget();
	virtual ~RenderTarget();

public:
	void Update(const Time deltaTime);
	
private:
	void initializeTexture();
	ID3D11Texture2D *_pRenderTargetTexture;
	ID3D11Texture2D *_pDepthStencilTexture;

public:
	void initializeMesh();
public:
	std::shared_ptr<MeshComponent> getMeshComponent();
private:
	std::shared_ptr<MeshComponent> _pMeshComponent;

public:
	ID3D11RenderTargetView* getRenderTargetView();
private:
	ID3D11RenderTargetView *_pRenderTargetView;

public:
	ID3D11ShaderResourceView* getShaderResouceView();
private:
	ID3D11ShaderResourceView *_pShaderResouceView;

public:
	ID3D11DepthStencilView* getDepthStencilView();
private:
	ID3D11DepthStencilView *_pDepthStencilView;

public:
	void Render();
};

#define __RENDER_TARGET_H__
#endif