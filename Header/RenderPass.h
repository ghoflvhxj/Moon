#pragma once
#ifndef __RENDER_PASS_H__
#define __RENDER_PASS_H__

class RenderTarget;
class TextureComponent;

class RenderPass
{
public:
	explicit RenderPass();
	~RenderPass();

public:
	void begin();
	void end();

public:
	void initializeRenderTarget(std::vector<std::shared_ptr<RenderTarget>> &renderTargetList);
private:
	std::vector<std::shared_ptr<RenderTarget>> _renderTargetList;

public:
	void initializeTexture(std::vector<std::shared_ptr<TextureComponent>> &textureList);
private:
	std::vector<std::shared_ptr<TextureComponent>> _textureList;

private:
	ID3D11RenderTargetView *_pOldRenderTargetView;
	ID3D11DepthStencilView *_pOldDepthStencilView;
};

#endif