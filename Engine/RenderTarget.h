#pragma once
#include "Include.h"

using namespace DirectX;

class MTexture;

enum class ERenderTargetType
{
    None,
    Default,
    Depth,
    Normal,
    Light,
    Bool
};

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
    ERenderTargetType Type;

	static const FRenderTagetInfo GetDefault(uint32 InWidth, uint32 InHeight);
	static const FRenderTagetInfo GetCube();
};

class MRenderTarget
{
public:
	explicit MRenderTarget();
	virtual ~MRenderTarget();
	
public:
	std::shared_ptr<MTexture> AsTexture();
protected:
    std::shared_ptr<MTexture> RenderTargetTexture;	// uniqueptr로 변경하기
    std::shared_ptr<MTexture> DepthStencilTexture;	// uniqueptr로 변경하기

public:
	void initializeTexture(const FRenderTagetInfo& InRenderTargetInfo);
protected:
    FRenderTagetInfo RenderTargetInfo;

public:
    void UpdateResolution(float InWidth, float InHeight);

private:
    DXGI_FORMAT GetFormat(const ERenderTargetType InRenderTargetType) const;

public:
	ID3D11RenderTargetView* AsRenderTargetView();
private:
	ID3D11RenderTargetView *_pRenderTargetView;

public:
	ID3D11DepthStencilView* getDepthStencilView();
private:
	ID3D11DepthStencilView *_pDepthStencilView;
};