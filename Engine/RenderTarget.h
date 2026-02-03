#pragma once
#include "Include.h"

using namespace DirectX;

class MTexture;

enum class ERenderTargetType
{
    None,
    Default,
    Diffuse,
    Depth  ,
    LinearDepth,
    Normal,
    Light,
    Bool,
    SingleFloat16,  // 이름은 임시
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
    RenderTargetView,
    ShaderResourceView,
};

class MRenderTarget
{
public:
	explicit MRenderTarget();
	virtual ~MRenderTarget();
	
    /***********************************
        포인트 라이트 쉐도우 맵의 경우 선형 뎁스 저장 해야함
        렌더 타겟이 필요할 뿐만 아니라, 렌더링 시 깊이 판별을 위한 깊이 스텐실도 필요함
    ***********************************/
public:
	std::shared_ptr<MTexture> AsTexture();
protected:
    std::shared_ptr<MTexture> RenderTargetTexture = nullptr;
    std::shared_ptr<MTexture> DepthStencilTexture = nullptr;

public:
	void initializeTexture(const FRenderTagetInfo& InRenderTargetInfo);
    const FRenderTagetInfo& GetRenderTargetInfo() const { return RenderTargetInfo; }
protected:
    FRenderTagetInfo RenderTargetInfo;

public:
    void UpdateResolution(float InWidth, float InHeight);

private:
    DXGI_FORMAT GetFormat(EDXResourceType InViewType, ERenderTargetType InRenderTargetType) const;
    DXGI_FORMAT GetDepthStencilFormat(EDXResourceType InViewType, ERenderTargetType InRenderTargetType) const;

public:
	ID3D11RenderTargetView* AsRenderTargetView();
private:
	ID3D11RenderTargetView *_pRenderTargetView;

public:
	ID3D11DepthStencilView* getDepthStencilView();
private:
	ID3D11DepthStencilView *_pDepthStencilView;
};