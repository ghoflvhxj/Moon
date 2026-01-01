#pragma once

#include "Include.h"
#include "Module/Module.h"
#include "EngineException.h"

#include <wrl/client.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

// DirectXTK
#include "DirectXTK/SpriteFont.h"

#include "Vertex.h"
#include "ShaderManager.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

class VertexShader;
class PixelShader;
class MGeometryShader;
class MRenderTarget;
class StaticMesh;
class MMesh;
struct FMeshData;
struct FWorldRenderInfo;

using Microsoft::WRL::ComPtr;

enum class ESamplerFilter
{
	Point,
	Linear,
	Anisotropic,
	Count
};

struct FWindowRenderData
{
    ComPtr<IDXGISwapChain1> SwapChain;
    ComPtr<IDXGISwapChain3> SwapChain3;
    std::array<ComPtr<ID3D11RenderTargetView>, 2> RenderTargetViews = {};

    ComPtr<ID3D11Texture2D> DepthStencilTexture;
    ComPtr<ID3D11DepthStencilView> DepthStencilView;
    ComPtr<ID3D11ShaderResourceView> DepthStencilSRV;
};

/******************************************
하나의 메시 렌더링에 사용되는 버퍼를 저장함
******************************************/
struct FBuffers
{
    std::shared_ptr<MVertexBuffer> VertexBuffer = nullptr;
    std::shared_ptr<MIndexBuffer> IndexBuffer = nullptr;
    std::shared_ptr<MVertexBuffer> InstanceBuffer = nullptr;
};

/******************************************
메시 렌더링에 사용되는 버퍼를 저장함
******************************************/
struct FBufferContainer
{
    void AddBuffers(uint32 InIndex, const FBuffers& InBuffers)
    {
        VertexBuffers[InIndex] = InBuffers.VertexBuffer;
        IndexBuffers[InIndex] = InBuffers.IndexBuffer;
        InstanceBuffers[InIndex] = InBuffers.InstanceBuffer;
    }

    std::map<uint32, std::shared_ptr<MVertexBuffer>> VertexBuffers;
    std::map<uint32, std::shared_ptr<MIndexBuffer>> IndexBuffers;
    std::map<uint32, std::shared_ptr<MVertexBuffer>> InstanceBuffers;
};

class ENGINE_DLL GraphicDevice : public MModule
{
public:
	class ENGINE_DLL Exception : public EngineException
	{
	public:
		explicit Exception(const int line, const char *file, const HRESULT hr);

	public:
		static const std::string TranslateErrorCode(const HRESULT hr);
		virtual const char *what() const override;
		virtual const char *GetType() const override;

	public:
		const std::string GetErrorString() const;

	public:
		const HRESULT GetErrorCode() const;
	private:
		HRESULT m_hResult;
	};

public:
	explicit GraphicDevice();
	virtual ~GraphicDevice() = default;

	GraphicDevice(const GraphicDevice &ref) = delete;
	GraphicDevice(GraphicDevice &&rRef) = delete;
	GraphicDevice &operator=(const GraphicDevice &ref) = delete;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

public:
    void Begin(int32 InWindowID, uint32 InWidth, uint32 InHeight);
    void End();
    bool Refresh();
    void SetToDefault();
public:
    int32 GetCurrentWindowIndex() const { return WindowID; }
    std::tuple<uint32, uint32> GetViewportSize() const { return { Width, Height }; }
protected:
    int32 WindowID = 0;
    uint32 Width = 0;
    uint32 Height = 0;

public:
    void ClearRenderTarget(const std::shared_ptr<MRenderTarget>& InRenderTarget, DirectX::XMVECTORF32 InColor);

public:
    ID3D11DepthStencilView* GetDepthStencilView();
    ID3D11ShaderResourceView* GetDepthStencilResourceView();

public:
    // 엔진에 윈도우가 추가되면 호출됨. 스왑체인 등을 생성해 WindowRenderData에 저장함
    void AddWindow(const FWorldRenderInfo& InWorldRenderInfo);
    void UpdateWindowSize(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight);

public:
    void SetVertexShader(std::shared_ptr<VertexShader>& vertexShader);
    void SetPixelShader(std::shared_ptr<PixelShader>& pixelShader);
public:
    bool GetVertexShader(const std::wstring InPath, std::shared_ptr<VertexShader>& OutShader);
    bool GetPixelShader(const std::wstring InPath, std::shared_ptr<PixelShader>& OutShader);
    bool GetGeometryShader(const std::wstring InPath, std::shared_ptr<MGeometryShader>& OutShader);
    std::unique_ptr<MShaderManager>& GetShaderManager();
public:
    std::unique_ptr<MShaderManager> ShaderManager = nullptr;

	//-------------------------------------------------------------------------
	// State
public:
	ID3D11SamplerState* getSamplerState(ESamplerFilter SamplerFilter);
	ID3D11RasterizerState *getRasterizerState(const Graphic::FillMode eFillMode, const Graphic::CullMode eCullMode, bool bDepthBias = false);
	ID3D11DepthStencilState *getDepthStencilState(const uint32 InFlag);
	ID3D11BlendState *getBlendState(const Graphic::Blend eBlend);
private:
    bool BuildInputLayout();
    bool buildSamplerState();
    bool buildRasterizerState();
    bool buildDepthStencilState();
    bool buildBlendState();

private:
	std::vector<ID3D11SamplerState*>		SamplerStates;
	std::vector<ID3D11RasterizerState*>		RasterizeStates;
    ID3D11RasterizerState* DepthBiasRS = nullptr;
	std::map<uint32, ID3D11DepthStencilState*>	DepthStencilStates;
	std::vector<ID3D11BlendState*>			BlendStates;

private:
	const bool initializeDirectXTK();
public:
	std::unique_ptr<DirectX::SpriteBatch> _spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> _spriteFont;

public:
	ID3D11Device *getDevice();
private:
	ID3D11Device *m_pDevice;

public:
    void QueryStart(uint32 InIndex);
    void QueryFinish(uint32 InIndex);
    std::array<ComPtr<ID3D11Query>, 3> Query;
    std::array<std::array<ComPtr<ID3D11Query>, 12>, 3> Start;
    std::array<std::array<ComPtr<ID3D11Query>, 12>, 3> Finish;
    int Counter = 0;
    std::vector<std::wstring> RenderPassTimes;

public:
	ID3D11DeviceContext *getContext();
public:
	ID3D11DeviceContext *getImmediateContext();
	ID3D11DeviceContext *getDefferedContext();
    ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout; }
private:
	ID3D11DeviceContext *m_pImmediateContext;	// 즉시 문맥: 싱글 쓰레드용
	ID3D11DeviceContext *m_pDeferredContext;	// 지연 문맥: 멀티 쓰레드용 CreateDeferredContext로 생성한다
    ID3D11InputLayout *m_pInputLayout;
private:
    //std::vector<FWindowRenderData> WindowRenderDatas;
    std::map<uint32, FWindowRenderData> WindowRenderDatas;

private:
	D3D11_VIEWPORT _viewport;

/***************************************** 
버퍼를 관리하는 기능들
생성, 삭제(작업해야함), 얻기 기능이 있음
******************************************/
public:
    void GetBuffers(FBufferContainer& OutBuffers, const std::shared_ptr<MMesh>& InMesh);
    void GetBuffers(FBufferContainer& OutBuffers, const std::wstring InKey);
    void GetPrivateBuffers(FBufferContainer& OutBuffers, uint32 InPID);
    void BuildMeshBuffer(const std::wstring& InKey, const FMeshData& InMeshData, uint32 InIndex, bool bInDynamic = false);
    void BuildMeshBuffers(const std::wstring& InKey, const std::vector<FMeshData>& InMeshDatas);
    void BuildMeshBuffersFromComponent(int32 InPID, const std::shared_ptr<MMesh>& InMesh);
    // 한 메시를 여러번 그릴 때 공유할 버퍼를 만듬
    void BuildMeshSharedBuffers(const std::shared_ptr<MMesh>& InMesh);
    // 메시컴포넌트의 전용 버퍼를 만듬 ex). Cloth시뮬
    void BuildMeshPrivateBuffers(int32 InPID, const std::shared_ptr<MMesh>& InMesh);
    void MakeBuffer(FBuffers& OutBuffers, const FMeshData& InMeshData, bool bInDynamic = false);
protected:
    // 메시들이 공유할 버퍼를 저장함
    std::map<const std::wstring, FBufferContainer> SharedBuffers;
    // PrimitiveComponent의 전용 버퍼를 저장함
    std::map<uint32, FBufferContainer> PrivateBuffers;

    REFLECT(GraphicDevice)
};

#define FAILED_CHECK_THROW(hr) if((HRESULT)hr < 0) throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError());
#define FAILED_CHECK_THROW_MSG(hr, message) if((HRESULT)hr < 0) { assert(false && TEXT(message)); throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError()); }

