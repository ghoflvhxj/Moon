#pragma once
#ifndef __RENDER_TARGET_H__

class MTexture;

struct FRenderTagetInfo
{
	FRenderTagetInfo()
		: TextrueNum{ 1 }
		, Width{ 0 }
		, Height{ 0 }
		, bCube{ false }
	{
	}
	uint32 TextrueNum;
	uint32 Width;
	uint32 Height;
	bool bCube;

	static const FRenderTagetInfo GetDefault();
	static const FRenderTagetInfo GetCube();
};

class RenderTarget
{
public:
	explicit RenderTarget();
	explicit RenderTarget(const FRenderTagetInfo& RenderTargetInfo);
	virtual ~RenderTarget();
	
public:
	std::shared_ptr<MTexture> AsTexture();
private:
	void initializeTexture(const FRenderTagetInfo& RenderTargetInfo);
	std::shared_ptr<MTexture> RenderTargetTexture;	// uniqueptr�� �����ϱ�
	std::shared_ptr<MTexture> DepthStencilTexture;	// uniqueptr�� �����ϱ�

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