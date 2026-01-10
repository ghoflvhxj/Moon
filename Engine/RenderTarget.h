#pragma once
#include "Include.h"

using namespace DirectX;

class MTexture;

enum class ERenderTargetType
{
    None,
    Default = 1 << 0,
    Depth = 1 << 1,
    Normal = 1 << 2,
    Light = 1 << 3,
    Bool = 1 << 4
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
    uint32 TypeFlag = 0;

	static const FRenderTagetInfo GetDefault(uint32 InWidth, uint32 InHeight);
	static const FRenderTagetInfo GetCube();
};

enum class EDXResourceType
{
    Texture,
    View,
};

class MRenderTarget
{
public:
	explicit MRenderTarget();
	virtual ~MRenderTarget();
	
public:
	std::shared_ptr<MTexture> AsTexture();
protected:
    std::shared_ptr<MTexture> Texture = nullptr;

public:
	void initializeTexture(const FRenderTagetInfo& InRenderTargetInfo);
    const FRenderTagetInfo& GetRenderTargetInfo() const { return RenderTargetInfo; }
protected:
    FRenderTagetInfo RenderTargetInfo;

public:
    void UpdateResolution(float InWidth, float InHeight);

private:
    DXGI_FORMAT GetFormat(EDXResourceType InViewType, ERenderTargetType InRenderTargetType) const;

public:
	ID3D11RenderTargetView* AsRenderTargetView();
private:
	ID3D11RenderTargetView *_pRenderTargetView;

public:
	ID3D11DepthStencilView* getDepthStencilView();
private:
	ID3D11DepthStencilView *_pDepthStencilView;
};