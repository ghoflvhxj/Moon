#pragma once

/********************************************
 Buffer - 텍스쳐나 렌더 타겟이 관리함. Structured 버퍼는 이곳에서 관리
 SRV - 텍스쳐나 렌더 타겟이 관리함. Structured 버퍼의 SRV는 이곳에서 관리


*********************************************/

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
#include "Module/Graphic/StructuredBuffer.h"

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
    ComPtr<ID3D11ShaderResourceView> DepthSRV;
    ComPtr<ID3D11ShaderResourceView> StencilSRV;
};

/******************************************
 하나의 메시 렌더링에 사용되는 버퍼를 저장함
******************************************/
struct FMeshBuffers
{
    std::shared_ptr<MVertexBuffer> VertexBuffer = nullptr;
    std::shared_ptr<MIndexBuffer> IndexBuffer = nullptr;
    std::shared_ptr<MVertexBuffer> InstanceBuffer = nullptr;
};

/******************************************
 메시 렌더링에 사용되는 버퍼를 저장함
******************************************/
struct FMeshBufferContainer
{
    void AddBuffers(uint32 InIndex, const FMeshBuffers& InBuffers)
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
	virtual ~GraphicDevice();

	GraphicDevice(const GraphicDevice &ref) = delete;
	GraphicDevice(GraphicDevice &&rRef) = delete;
	GraphicDevice &operator=(const GraphicDevice &ref) = delete;

public:
    static bool bReverseDepth;
    static float GetNear();
    static float GetFar();

public:
	virtual bool Initialize() override;
	virtual void Release() override;

private:
    const bool initializeDirectXTK();
public:
    std::unique_ptr<SpriteBatch> _spriteBatch;
    std::unique_ptr<SpriteFont> _spriteFont;

    void Test()
    {
        _spriteBatch->Begin();

        XMVECTOR Pos = XMVectorSet(100.f, 100.f, 0.f, 0.f);
        _spriteFont->DrawString(_spriteBatch.get(), TEXT("Hello World, 안녕하세요"), Pos, Colors::White, 0.f, g_XMZero);

        _spriteBatch->End();
    }

public:
    ID3D11Device* getDevice();
    ID3D11DeviceContext* getContext();
    ID3D11DeviceContext* getImmediateContext();
    ID3D11DeviceContext* getDefferedContext();
    ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout; }
private:
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pImmediateContext;	// 즉시 문맥: 싱글 쓰레드용
    ID3D11DeviceContext* m_pDeferredContext;	// 지연 문맥: 멀티 쓰레드용 CreateDeferredContext로 생성한다
    ID3D11InputLayout* m_pInputLayout;

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

    /***********************************************
        윈도우 관련 기능 및 디폴트 뷰
    ***********************************************/
public:
    void AddWindow(const FWorldRenderInfo& InWorldRenderInfo);
    void UpdateWindowSize(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight, bool InFullScreen);
public:
    ID3D11DepthStencilView* GetDepthStencilView();
    ID3D11ShaderResourceView* GetDepthResourceView();
    ID3D11ShaderResourceView* GetStencilResourceView();

    /***********************************************
        컨텍스트 기능 - 쉐이더
    ***********************************************/
public:
    void ClearRenderTarget(const std::shared_ptr<MRenderTarget>& InRenderTarget, DirectX::XMVECTORF32 InColor);

    /***********************************************
        컨텍스트 기능 - 드로잉
    ***********************************************/
public:
    void Draw(const std::shared_ptr<MVertexBuffer>& InVertexBuffer, const std::shared_ptr<MIndexBuffer>& InIndexBuffer);
    void DrawInstance(const std::shared_ptr<MVertexBuffer>& InVertexBuffer, const std::shared_ptr<MIndexBuffer>& InIndexBuffer, const std::shared_ptr<MVertexBuffer>& InInstanceBuffer);

    /***********************************************
        컨텍스트 기능 - 쉐이더
    ***********************************************/
public:
    void PSSet(std::shared_ptr<PixelShader>& pixelShader);
    void PSSetSRV(UINT InSlot, uint32 InSRVID);
public:
    void VSSet(std::shared_ptr<VertexShader>& vertexShader);
public:
    bool GetVertexShader(const std::wstring InPath, std::shared_ptr<VertexShader>& OutShader);
    bool GetPixelShader(const std::wstring InPath, std::shared_ptr<PixelShader>& OutShader);
    bool GetGeometryShader(const std::wstring InPath, std::shared_ptr<MGeometryShader>& OutShader);
    std::unique_ptr<MShaderManager>& GetShaderManager();
public:
    std::unique_ptr<MShaderManager> ShaderManager = nullptr;

    /***********************************************
        상태 관리
    ***********************************************/
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
    ID3D11RasterizerState* DepthPrePassRS = nullptr;
	std::map<uint32, ID3D11DepthStencilState*>	DepthStencilStates;
    ComPtr<ID3D11DepthStencilState> LinearDepthStencilState;
	std::vector<ID3D11BlendState*>			BlendStates;
public:
    ComPtr<ID3D11RasterizerState> ShadowDepthRS = nullptr;

    /***********************************************
        DepthStencil 설정
    ***********************************************/
public:
    void LinearDepthStencil(); // 이름은 임시
    void RSDepthPre();

    /***********************************************
        GPU 프로파일링
    ***********************************************/
public:
    void QueryStart(uint32 InIndex);
    void QueryFinish(uint32 InIndex);
    std::array<ComPtr<ID3D11Query>, 3> Query;
    // 렌더패스 - 쿼리 쌍
    std::array<std::array<ComPtr<ID3D11Query>, 15>, 3> Start;
    std::array<std::array<ComPtr<ID3D11Query>, 15>, 3> Finish;
    int Counter = 0;
    std::vector<std::wstring> RenderPassTimes;

private:
    std::map<uint32, FWindowRenderData> WindowRenderDatas;

private:
	D3D11_VIEWPORT _viewport;

    /******************************************
        메시 관련 버퍼 관리
    ******************************************/
public:
    void GetBuffers(FMeshBufferContainer& OutBuffers, const std::shared_ptr<MMesh>& InMesh);
    void GetBuffers(FMeshBufferContainer& OutBuffers, const std::wstring InKey);
    void GetPrivateBuffers(FMeshBufferContainer& OutBuffers, uint32 InPID);
    void BuildMeshBuffer(const std::wstring& InKey, const FMeshData& InMeshData, uint32 InIndex, bool bInDynamic = false);
    void BuildMeshBuffers(const std::wstring& InKey, const std::vector<FMeshData>& InMeshDatas);
    void BuildMeshBuffersFromComponent(int32 InPID, const std::shared_ptr<MMesh>& InMesh);
    // 한 메시를 여러번 그릴 때 공유할 버퍼를 만듬
    void BuildMeshSharedBuffers(const std::shared_ptr<MMesh>& InMesh);
    // 메시컴포넌트의 전용 버퍼를 만듬 ex). Cloth시뮬
    void BuildMeshPrivateBuffers(int32 InPID, const std::shared_ptr<MMesh>& InMesh);
    void MakeBuffer(FMeshBuffers& OutBuffers, const FMeshData& InMeshData, bool bInDynamic = false);
protected:
    // 메시들이 공유할 버퍼를 저장함
    std::map<const std::wstring, FMeshBufferContainer> SharedBuffers;
    // PrimitiveComponent의 전용 버퍼를 저장함
    std::map<uint32, FMeshBufferContainer> PrivateBuffers;

    /******************************************
        버퍼, SRV 관리
        TODO. 삭제는 어떻게 해야할지 생각해봐야 함.
    ******************************************/
public:
    MStructuredBuffer AddStructuredBuffer(const void* InData, UINT InDataSize, UINT InElementNum, UINT InElementSize);
    void UpdateStructuredBuffer(MStructuredBuffer& InBuffer, const void* InData, UINT InDataSize, UINT InElementNum, UINT InElementSize);
private:
    ComPtr<ID3D11Buffer> CreateStructuredBuffer(const void* InData, UINT InDataSize, UINT InElementNum, UINT InElementSize);
    ComPtr<ID3D11ShaderResourceView> CreateStructuredBufferSRV(ID3D11Buffer* InBuffer, UINT InElementNum, UINT InElementSize);
private:
    ID3D11Buffer* GetRawBuffer(MStructuredBuffer& InBuffer);
protected:
    std::map<uint32, ComPtr<ID3D11Buffer>> StructuredBuffers;
    std::map<uint32, ComPtr<ID3D11ShaderResourceView>> StructuredBufferSRVs;
private:
    uint32 BufferCounter = 0;
    uint32 SRVCounter = 0;
    std::map<uint32, uint32> BufferToSRV;


    REFLECT(GraphicDevice)
};

#define FAILED_CHECK_THROW(hr) if((HRESULT)hr < 0) throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError());
#define FAILED_CHECK_THROW_MSG(hr, message) if((HRESULT)hr < 0) { assert(false && TEXT(message)); throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError()); }

