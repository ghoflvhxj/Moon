#include "GraphicDevice.h"
#include "MoonEngine.h"

#include "MainGameSetting.h"
#include "Window.h"

#include "Vertex.h"
#include "InputLayout.h"
#include "RenderTarget.h"
#include "ShaderLoader.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Mesh/StaticMesh/StaticMesh.h"


#pragma comment(lib, "dxgi.lib")


using namespace DirectX;
using namespace Graphic;

GraphicDevice::GraphicDevice()
	: m_pDevice{ nullptr }
	, m_pImmediateContext{ nullptr }
	, m_pDeferredContext{ nullptr }
	, _spriteBatch{ nullptr }
	, _spriteFont{ nullptr }

	, m_pInputLayout{ nullptr }
	, _viewport{ }
{
    bManualReleaseRequired = true;
}

bool GraphicDevice::Initialize()
{
    Super::Initialize();

    GetEngine()->GetOnWorldAddedDelegate().Add(this, &GraphicDevice::AddWindow);

    ComPtr<IDXGIFactory2> factory = nullptr;
    UINT flags = 0;
    HRESULT hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        // 폴백: CreateDXGIFactory1 사용해 볼 수도 있음
        ComPtr<IDXGIFactory> factory1;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory1));
        if (SUCCEEDED(hr)) {
            factory1.As(&factory); // 가능하면 IDXGIFactory2로 업캐스트 시도
        }
    }

	// 장치
    FAILED_CHECK_THROW(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &m_pDevice, nullptr, &m_pImmediateContext
    ));

	// 뷰포트
	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	_viewport.Width = g_pSetting->getResolutionWidth<FLOAT>();
	_viewport.Height = g_pSetting->getResolutionHeight<FLOAT>();
	_viewport.MaxDepth = 1.f;
	_viewport.MinDepth = 0.f;

	m_pImmediateContext->RSSetViewports(1, &_viewport);

    initializeDirectXTK();

    buildSamplerState();
    buildRasterizerState();
    buildDepthStencilState();
    buildBlendState();
    BuildInputLayout();

	return true;
}

void GraphicDevice::Release()
{
    Super::Release();

    ShaderManager.reset();
    for (auto& Shader : MShader::GetSharedConstantBuffers())
    {
        Shader.reset();
    }

    _spriteFont.reset();
    _spriteBatch.reset();

    SafeReleaseArray(SamplerStates);
    SafeReleaseArray(RasterizeStates);
    SafeReleaseArray(BlendStates);
    for (auto& [Flag, DepthStencilState] : DepthStencilStates)
    {
        SafeRelease(DepthStencilState);
    }

    SafeRelease(DepthBiasRS);

    SafeRelease(m_pInputLayout);

    WindowRenderDatas.clear();

#ifdef MULTITHREAD
    m_pDeferredContext->ClearState();
    m_pDeferredContext->Flush();
    SafeRelease(m_pDeferredContext);
#else
    if (m_pImmediateContext)
    {
        m_pImmediateContext->ClearState();
        m_pImmediateContext->Flush();
    }
    SafeRelease(m_pImmediateContext);
#endif

    SafeRelease(m_pDevice);

    IDXGIDebug1* debug = nullptr;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug));
    OutputDebugStringW(TEXT("----------------ReportLiveObjectsBegin-------------------\r\n"));
    debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugStringW(TEXT("----------------ReportLiveObjectsEnd-------------------\r\n"));
    SafeRelease(debug);
}

bool GraphicDevice::BuildInputLayout()
{
    ShaderManager = std::make_unique<MShaderManager>();
    ShaderLoader shaderLoader;
    shaderLoader.loadShaderFiles(ShaderManager);

    // 디폴트 InputLayout
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescList;
        getDesc(inputDescList);

        if (ShaderManager)
        {
            if (ID3DBlob* pBlob = ShaderManager->getVertexShaderBlob(TEXT("TexVertexShader.cso")))
            {
                UINT elementCount = static_cast<UINT>(inputDescList.size());
                FAILED_CHECK_THROW(m_pDevice->CreateInputLayout(inputDescList.data(), elementCount, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout));

                m_pImmediateContext->IASetInputLayout(m_pInputLayout);
            }
        }
    }

    //{
    //    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescList;
    //    Graphic::VERTEX_SIMPLE::getDesc(inputDescList);

    //    ID3DBlob* pBlob = ShaderManager->getVertexShaderBlob(TEXT("SimpleVertexShader.cso"));

    //    UINT elementCount = static_cast<UINT>(inputDescList.size());
    //    FAILED_CHECK_THROW(m_pDevice->CreateInputLayout(inputDescList.data(), elementCount, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &SimpleLayout));
    //}


	return true;
}

void GraphicDevice::ClearRenderTarget(const std::shared_ptr<MRenderTarget>& InRenderTarget, DirectX::XMVECTORF32 InColor)
{
    getContext()->ClearRenderTargetView(InRenderTarget->AsRenderTargetView(), reinterpret_cast<const float*>(&InColor));
    getContext()->ClearDepthStencilView(InRenderTarget->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
}

ID3D11DepthStencilView* GraphicDevice::GetDepthStencilView()
{
    const FWindowRenderData& Test = WindowRenderDatas[WindowID];
    return Test.DepthStencilView.Get();
}

void GraphicDevice::AddWindow(const FWorldRenderInfo& InWorldRenderInfo)
{
    auto& Window = InWorldRenderInfo.DstWindow;
    Window->GetOnViewportSizeChangedDelegate().Add(this, &GraphicDevice::UpdateWindowSize);

    ComPtr<IDXGIFactory2> factory = nullptr;
    UINT flags = 0;
    HRESULT hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        // 폴백: CreateDXGIFactory1 사용해 볼 수도 있음
        ComPtr<IDXGIFactory> factory1;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory1));
        if (SUCCEEDED(hr)) {
            factory1.As(&factory); // 가능하면 IDXGIFactory2로 업캐스트 시도
        }
    }

    FWindowRenderData NewWindowRenderData = {};

    // 스왑체인
    DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
    swapDesc.Width = Window->GetWidth<UINT>();
    swapDesc.Height = Window->GetHeight<UINT>();
    swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = 2;
    swapDesc.Scaling = DXGI_SCALING_NONE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapDesc.Flags = 0;

    FAILED_CHECK_THROW(factory->CreateSwapChainForHwnd(m_pDevice, InWorldRenderInfo.DstWindow->getHandle(), &swapDesc, nullptr, nullptr, NewWindowRenderData.SwapChain.GetAddressOf()));
    NewWindowRenderData.SwapChain->QueryInterface(IID_PPV_ARGS(NewWindowRenderData.SwapChain3.GetAddressOf()));

    // 렌더 타겟 뷰 생성
    std::array<ID3D11Texture2D*, 2> SawpChainBuffers = {};
    for (uint32 i = 0; i < GetSize(SawpChainBuffers); ++i)
    {
        FAILED_CHECK_THROW(NewWindowRenderData.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SawpChainBuffers[i]));
        FAILED_CHECK_THROW(m_pDevice->CreateRenderTargetView(SawpChainBuffers[i], nullptr, NewWindowRenderData.RenderTargetViews[i].GetAddressOf()));
    }
    SafeReleaseArray(SawpChainBuffers);

    // 깊이 스텐실 뷰 생성
    D3D11_TEXTURE2D_DESC depthStencilDesc = { };
    depthStencilDesc.Width = Window->GetWidth<UINT>();
    depthStencilDesc.Height = Window->GetHeight<UINT>();
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, NewWindowRenderData.DepthStencilBuffer.GetAddressOf());
    FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilView(NewWindowRenderData.DepthStencilBuffer.Get(), nullptr, NewWindowRenderData.DepthStencilView.GetAddressOf()));

    WindowRenderDatas[InWorldRenderInfo.DstWindow->GetID()] = NewWindowRenderData;
}

void GraphicDevice::UpdateWindowSize(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight)
{
    std::cout << "Update Window Size" << std::endl;

    GetPostLoopDelegate().Add([&, InWindowID, InNewWidth, InNewHeight]() {
        //getContext()->ClearState();

        UINT Width = static_cast<UINT>(InNewWidth);
        UINT Height = static_cast<UINT>(InNewHeight);

        auto& WindowRenderData = WindowRenderDatas[InWindowID];

        WindowRenderData.DepthStencilBuffer.Reset();
        WindowRenderData.DepthStencilView.Reset();
        WindowRenderData.RenderTargetViews[0].Reset();
        WindowRenderData.RenderTargetViews[1].Reset();

        HRESULT HR = WindowRenderData.SwapChain->ResizeBuffers(0, InNewWidth, InNewHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        if (HR == S_OK)
        {
            // 렌더 타겟 뷰 생성
            std::array<ID3D11Texture2D*, 2> SawpChainBuffers = {};
            for (uint32 i = 0; i < GetSize(SawpChainBuffers); ++i)
            {
                FAILED_CHECK_THROW(WindowRenderData.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SawpChainBuffers[i]));
                D3D11_TEXTURE2D_DESC De = {};
                SawpChainBuffers[i]->GetDesc(&De);

                FAILED_CHECK_THROW(m_pDevice->CreateRenderTargetView(SawpChainBuffers[i], nullptr, WindowRenderData.RenderTargetViews[i].GetAddressOf()));
            }
            SafeReleaseArray(SawpChainBuffers);

            D3D11_RENDER_TARGET_VIEW_DESC Desc = {};
            WindowRenderData.RenderTargetViews[0]->GetDesc(&Desc);

            // 깊이 스텐실 버퍼, 뷰 생성
            D3D11_TEXTURE2D_DESC depthStencilDesc = { };
            depthStencilDesc.Width = Width;
            depthStencilDesc.Height = Height;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            depthStencilDesc.SampleDesc.Count = 1;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.CPUAccessFlags = 0;
            depthStencilDesc.MiscFlags = 0;

            m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, WindowRenderData.DepthStencilBuffer.GetAddressOf());
            FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilView(WindowRenderData.DepthStencilBuffer.Get(), nullptr, WindowRenderData.DepthStencilView.GetAddressOf()));
        }
    });
}

bool GraphicDevice::GetVertexShader(const std::wstring InPath, std::shared_ptr<VertexShader>& OutShader)
{
    return ShaderManager->getVertexShader(InPath.c_str(), OutShader);
}

bool GraphicDevice::GetPixelShader(const std::wstring InPath, std::shared_ptr<PixelShader>& OutShader)
{
    return ShaderManager->getPixelShader(InPath.c_str(), OutShader);
}

bool GraphicDevice::GetGeometryShader(const std::wstring InPath, std::shared_ptr<MGeometryShader>& OutShader)
{
    return ShaderManager->getGeometryShader(InPath.c_str(), OutShader);
}

std::unique_ptr<MShaderManager>& GraphicDevice::GetShaderManager()
{
    return ShaderManager;
}

bool GraphicDevice::Refresh()
{
	assert(m_pDevice);
	assert(m_pImmediateContext);

	return true;
}

void GraphicDevice::SetToDefault()
{
    // 인풋 레이아웃은 수정할 일이 없긴 함
    // g_pGraphicDevice->getContext()->IASetInputLayout(g_pGraphicDevice->GetInputLayout());

    const FWindowRenderData& WindowRenderData = WindowRenderDatas[WindowID];
    UINT BufferIndex = WindowRenderData.SwapChain3->GetCurrentBackBufferIndex();

    // 쉐이더 리소스 뷰 해제
    uint32 ResorceViewNum = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    std::vector<ID3D11ShaderResourceView*> RowResourceViews(ResorceViewNum, nullptr);
    getContext()->PSSetShaderResources(0, ResorceViewNum, RowResourceViews.data());

    // 렌더 타겟
    uint32 RenderTargetNum = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
    std::vector<ID3D11RenderTargetView*> restoreRenderTargetViewArray(RenderTargetNum, nullptr);
    restoreRenderTargetViewArray[0] = WindowRenderData.RenderTargetViews[BufferIndex].Get();
    getContext()->OMSetRenderTargets(static_cast<UINT>(RenderTargetNum), restoreRenderTargetViewArray.data(), WindowRenderData.DepthStencilView.Get());

    D3D11_VIEWPORT Viewport;
    Viewport.Width = static_cast<FLOAT>(Width);
    Viewport.Height = static_cast<FLOAT>(Height);
    Viewport.TopLeftX = 0.f;
    Viewport.TopLeftY = 0.f;
    Viewport.MinDepth = 0.f;
    Viewport.MaxDepth = 1.f;
    getContext()->RSSetViewports(1, &Viewport);

    UINT RectNum = 1;
    D3D11_RECT Rect = {};
    Rect.left = 0;
    Rect.top = 0;
    Rect.right = g_pSetting->getResolutionWidth<LONG>();
    Rect.bottom = g_pSetting->getResolutionHeight<LONG>();
    getContext()->RSSetScissorRects(RectNum, &Rect);
}

void GraphicDevice::Begin(int32 InWindowID, uint32 InWidth, uint32 InHeight)
{
    auto& Iter = WindowRenderDatas.find(InWindowID);
    if (Iter == WindowRenderDatas.end())
    {
        // HWND보다는 공통된 int 타입으로 관리하고 싶음
        WindowID = -1;
        return;
    }

    WindowID = InWindowID;

    if (Width != InWidth || Height != InHeight)
    {
        bResized = true;
        Width = InWidth;
        Height = InHeight;
    }

    const FWindowRenderData& WindowRenderData = Iter->second;

    UINT BufferIndex = WindowRenderData.SwapChain3->GetCurrentBackBufferIndex();
    getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    getContext()->OMSetRenderTargets(1, WindowRenderData.RenderTargetViews[BufferIndex].GetAddressOf(), WindowRenderData.DepthStencilView.Get());

    std::array<ID3D11Texture2D*, 2> SawpChainBuffers = {};
    FAILED_CHECK_THROW(WindowRenderData.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SawpChainBuffers[BufferIndex]));
    D3D11_TEXTURE2D_DESC De = {};
    SawpChainBuffers[BufferIndex]->GetDesc(&De);
    SafeRelease(SawpChainBuffers[BufferIndex]);

    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = static_cast<FLOAT>(InWidth);
    Viewport.Height = static_cast<FLOAT>(InHeight);
    Viewport.TopLeftX = 0.f;
    Viewport.TopLeftY = 0.f;
    Viewport.MinDepth = 0.f;
    Viewport.MaxDepth = 1.f;
    getContext()->RSSetViewports(1, &Viewport);
}

void GraphicDevice::End()
{
    if (WindowID == -1)
    {
        return;
    }

    bResized = false;

    const FWindowRenderData& Test = WindowRenderDatas[WindowID];
    Test.SwapChain3->Present(0u, 0u);

    UINT BufferIndex = Test.SwapChain3->GetCurrentBackBufferIndex();
    getContext()->ClearRenderTargetView(Test.RenderTargetViews[BufferIndex].Get(), reinterpret_cast<const float*>(&EngineColors::Blue));
    getContext()->ClearDepthStencilView(Test.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
}

bool GraphicDevice::buildRasterizerState()
{
	RasterizeStates.reserve(CastValue<uint32>(Graphic::FillMode::Count) + CastValue<uint32>(Graphic::CullMode::Count));
	ID3D11RasterizerState *pRasterizerState = nullptr;

	//-------------------------------------------------------------------------------------
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rd.FrontCounterClockwise = FALSE;
	rd.DepthBias = 0;
	rd.SlopeScaledDepthBias = 0.f;
	rd.DepthBiasClamp = 0.f;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = TRUE;
	rd.MultisampleEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;

    {
        rd.DepthBias = 100000;
        rd.SlopeScaledDepthBias = 1.f;
        FAILED_CHECK_THROW(m_pDevice->CreateRasterizerState(&rd, &DepthBiasRS));
        rd.DepthBias = 0;
        rd.SlopeScaledDepthBias = 0.f;
    }

	//-------------------------------------------------------------------------------------
	for (uint32 fillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME; fillMode <= D3D11_FILL_MODE::D3D11_FILL_SOLID; ++fillMode)
	{
		for (uint32 cullMode = D3D11_CULL_MODE::D3D11_CULL_NONE; cullMode <= D3D11_CULL_MODE::D3D11_CULL_BACK; ++cullMode)
		{
			rd.FillMode = D3D11_FILL_MODE(fillMode);
			rd.CullMode = D3D11_CULL_MODE(cullMode);

            FAILED_CHECK_THROW(m_pDevice->CreateRasterizerState(&rd, &pRasterizerState));
            RasterizeStates.push_back(pRasterizerState);
		}
	}

	return true;
}

ID3D11RasterizerState *GraphicDevice::getRasterizerState(const Graphic::FillMode eFillMode, const Graphic::CullMode eCullMode, bool bDepthBias)
{
    if (bDepthBias)
    {
        return DepthBiasRS;
    }

	return RasterizeStates[(EnumToIndex(eFillMode) * CastValue<uint32>(Graphic::CullMode::Count)) + EnumToIndex(eCullMode)];
}

bool GraphicDevice::buildDepthStencilState()
{
	ID3D11DepthStencilState *pDepthStencilState = nullptr;

	//-------------------------------------------------------------------------------------
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = TRUE;
	dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	dsd.BackFace = dsd.FrontFace;

	//-------------------------------------------------------------------------------------
    {
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&dsd, &pDepthStencilState));
        DepthStencilStates[EnumToFlag(EDepthStencilMode::DepthEnable, EDepthStencilMode::StencilEnable)] = pDepthStencilState;
    }

    //-------------------------------------------------------------------------------------
    {
        D3D11_DEPTH_STENCIL_DESC Copy = dsd;
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&Copy, &pDepthStencilState));
        Copy.StencilReadMask = 0xFF;
        Copy.StencilWriteMask = 0x00;
        Copy.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        Copy.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        Copy.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        Copy.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        Copy.BackFace = Copy.FrontFace;
        DepthStencilStates[EnumToFlag(EDepthStencilMode::DepthEnable, EDepthStencilMode::StencilReadMask)] = pDepthStencilState;
    }

    //-------------------------------------------------------------------------------------
    {
        D3D11_DEPTH_STENCIL_DESC Copy = dsd;
        Copy.StencilEnable = FALSE;
        Copy.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;
        Copy.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        Copy.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        Copy.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&Copy, &pDepthStencilState));
        DepthStencilStates[EnumToFlag(EDepthStencilMode::DepthEnable, EDepthStencilMode::StencilDisable)] = pDepthStencilState;
    }

	//-------------------------------------------------------------------------------------
    {
        D3D11_DEPTH_STENCIL_DESC Copy = dsd;
        Copy.DepthEnable = FALSE;
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&Copy, &pDepthStencilState));
        DepthStencilStates[EnumToFlag(EDepthStencilMode::DepthDisable, EDepthStencilMode::StencilEnable)] = pDepthStencilState;
    }

    //-------------------------------------------------------------------------------------
    {
        D3D11_DEPTH_STENCIL_DESC Copy = dsd;
        Copy.DepthEnable = FALSE;
        Copy.StencilReadMask = 0xFF;
        Copy.StencilWriteMask = 0x00;
        Copy.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        Copy.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        Copy.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        Copy.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        Copy.BackFace = Copy.FrontFace;
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&Copy, &pDepthStencilState));
        DepthStencilStates[EnumToFlag(EDepthStencilMode::DepthDisable, EDepthStencilMode::StencilReadMask)] = pDepthStencilState;
    }

    //-------------------------------------------------------------------------------------
    {
        D3D11_DEPTH_STENCIL_DESC Copy = dsd;
        Copy.DepthEnable = FALSE;
        Copy.StencilEnable = FALSE;
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&Copy, &pDepthStencilState));
        DepthStencilStates[EnumToFlag(EDepthStencilMode::DepthDisable, EDepthStencilMode::StencilDisable)] = pDepthStencilState;
    }

	return true;
}

ID3D11DepthStencilState *GraphicDevice::getDepthStencilState(const uint32 InFlag)
{
	return DepthStencilStates[InFlag];
}

bool GraphicDevice::buildBlendState()
{
	D3D11_BLEND_DESC bd = {};
	ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
	ID3D11BlendState *pBlendState = nullptr;

	//-------------------------------------------------------------------------------------
	bd.AlphaToCoverageEnable			= FALSE;
	bd.IndependentBlendEnable			= FALSE;
	bd.RenderTarget[0].BlendEnable		= FALSE;
	bd.RenderTarget[0].SrcBlend			= D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend		= D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOp			= D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha	= D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha	= D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha		= D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	for (uint32 i = 1; i < 8; ++i)
	{
		bd.RenderTarget[i] = bd.RenderTarget[0];
	}

	//-------------------------------------------------------------------------------------
	FAILED_CHECK_THROW(m_pDevice->CreateBlendState(&bd, &pBlendState));
	BlendStates.push_back(pBlendState);

	//-------------------------------------------------------------------------------------
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	for (uint32 i = 1; i < 8; ++i)
	{
		bd.RenderTarget[i] = bd.RenderTarget[0];
	}
	FAILED_CHECK_THROW(m_pDevice->CreateBlendState(&bd, &pBlendState));
	BlendStates.push_back(pBlendState);

	return true;
}

ID3D11BlendState *GraphicDevice::getBlendState(const Graphic::Blend eBlend)
{
	return BlendStates[EnumToIndex(eBlend)];
}

bool GraphicDevice::buildSamplerState()
{
	auto CreateSamplerLambda = [this](D3D11_SAMPLER_DESC& samplerDesc)
	{
		ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK(m_pDevice->CreateSamplerState(&samplerDesc, &pSamplerState));
		SamplerStates.emplace_back(pSamplerState);
	};

	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.BorderColor[0] = 1.f;
	SamplerDesc.BorderColor[1] = 1.f;
	SamplerDesc.BorderColor[2] = 1.f;
	SamplerDesc.BorderColor[3] = 1.f;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MaxAnisotropy = 1u;
	SamplerDesc.MinLOD = 0.f;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	SamplerDesc.MipLODBias = 0.f;

	// Point
	SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
	CreateSamplerLambda(SamplerDesc);

	// Linear
	SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	CreateSamplerLambda(SamplerDesc);

	// Anisotropic
	SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
	SamplerDesc.MaxAnisotropy = 1u;
	CreateSamplerLambda(SamplerDesc);

	// Comparison MIN MAG LINEAR MIP POINT
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	SamplerDesc.BorderColor[0] = 1.f;
	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	SamplerDesc.MaxAnisotropy = 1u;
	SamplerDesc.MipLODBias = 0.f;

    {
        ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState), false);
        SamplerStates.emplace_back(pSamplerState);
        m_pImmediateContext->PSSetSamplers(1, 1, &pSamplerState);
    }

    {
        SamplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
        ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState), false);
        SamplerStates.emplace_back(pSamplerState);
        m_pImmediateContext->PSSetSamplers(2, 1, &pSamplerState);
    }

	m_pImmediateContext->PSSetSamplers(0, 1, &SamplerStates[0]);

	return true;
}

ID3D11SamplerState* GraphicDevice::getSamplerState(ESamplerFilter SamplerFilter)
{
	if (EnumToIndex(SamplerFilter) < SamplerStates.size())
	{
		return SamplerStates[EnumToIndex(SamplerFilter)];
	}

	return nullptr;
}

const bool GraphicDevice::initializeDirectXTK()
{
	//_spriteBatch = std::make_unique<SpriteBatch>(m_pImmediateContext);
	//_spriteFont = std::make_unique<SpriteFont>(m_pDevice, TEXT("TestFont.spritefont"));

	return true;
}

void GraphicDevice::SetVertexShader(std::shared_ptr<VertexShader> &vertexShader)
{
	getContext()->VSSetShader(vertexShader->getRaw(), nullptr, 0);
}

void GraphicDevice::SetPixelShader(std::shared_ptr<PixelShader> &pixelShader)
{
	getContext()->PSSetShader(pixelShader->getRaw(), nullptr, 0);
}

ID3D11Device *GraphicDevice::getDevice()
{
	return m_pDevice;
}

ID3D11DeviceContext *GraphicDevice::getContext()
{
#ifndef MULTITHREAD
	return getImmediateContext();
#else
	return getDefferedContext();
#endif
}

ID3D11DeviceContext *GraphicDevice::getImmediateContext()
{
	return m_pImmediateContext;
}

ID3D11DeviceContext *GraphicDevice::getDefferedContext()
{
	return m_pDeferredContext;
}

void GraphicDevice::GetBuffers(FBufferContainer& OutBuffers, const std::shared_ptr<MMesh>& InMesh)
{
    const std::wstring& AssetPath = InMesh->GetAssetPath();
    GetBuffers(OutBuffers, AssetPath);
}

void GraphicDevice::GetBuffers(FBufferContainer& OutBuffers, const std::wstring InKey)
{
    auto& Iter = SharedBuffers.find(InKey);

    if (SharedBuffers.end() == Iter)
    {
        return;
    }

    OutBuffers = Iter->second;
}

void GraphicDevice::GetPrivateBuffers(FBufferContainer& OutBuffers, uint32 InPID)
{
    auto& Iter = PrivateBuffers.find(InPID);

    if (PrivateBuffers.end() == Iter)
    {
        return;
    }

    OutBuffers = Iter->second;
}

void GraphicDevice::BuildMeshBuffer(const std::wstring& InKey, const FMeshData& InMeshData, uint32 InIndex, bool bInDynamic)
{
    FBuffers NewSharedBuffers = {};
    MakeBuffer(NewSharedBuffers, InMeshData, bInDynamic);
    SharedBuffers[InKey].AddBuffers(InIndex, NewSharedBuffers);
}

void GraphicDevice::BuildMeshBuffers(const std::wstring& InKey, const std::vector<FMeshData>& InMeshDatas)
{
    uint32 Num = GetSize(InMeshDatas);
    for (uint32 i = 0; i < Num; ++i)
    {
        BuildMeshBuffer(InKey, InMeshDatas[i], i);
    }
}

void GraphicDevice::BuildMeshBuffersFromComponent(int32 InPID, const std::shared_ptr<MMesh>& InMesh)
{
    BuildMeshSharedBuffers(InMesh);
    BuildMeshPrivateBuffers(InPID, InMesh);
}

void GraphicDevice::BuildMeshSharedBuffers(const std::shared_ptr<MMesh>& InMesh)
{
    if (InMesh == nullptr)
    {
        return;
    }

    const std::wstring& AssetPath = InMesh->GetAssetPath();
    auto& Iter = SharedBuffers.find(AssetPath);
    if (SharedBuffers.end() != Iter)
    {
        return;
    }

    for (uint32 i = 0; i < InMesh->GetMeshNum(); ++i)
    {
        const FMeshData& MeshData = InMesh->GetMeshData(i);

        FBuffers NewSharedBuffers = {};
        MakeBuffer(NewSharedBuffers, MeshData);
        SharedBuffers[AssetPath].AddBuffers(i, NewSharedBuffers);
    }
}

void GraphicDevice::BuildMeshPrivateBuffers(int32 InPID, const std::shared_ptr<MMesh>& InMesh)
{
    if (InMesh == nullptr)
    {
        return;
    }

    if (InPID == -1)
    {
        return;
    }

    auto& Iter = PrivateBuffers.find(InPID);
    if (PrivateBuffers.end() != Iter)
    {
        return;
    }

    for (uint32 i = 0; i < InMesh->GetMeshNum(); ++i)
    {
        const FMeshData& MeshData = InMesh->GetMeshData(i);

        // 클로딩 등으로 전용 버퍼가 필요한 경우
        if (InMesh->IsClothigMesh(i) == false)
        {
            continue;
        }

        FBuffers NewPrivateBuffers = {};
        MakeBuffer(NewPrivateBuffers, MeshData, true);
        PrivateBuffers[InPID].AddBuffers(i, NewPrivateBuffers);
    }
}

void GraphicDevice::MakeBuffer(FBuffers& OutBuffers, const FMeshData& InMeshData, bool bInDynamic)
{
    uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
    uint32 VertexNum = GetSize(InMeshData.Vertices);
    OutBuffers.VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, InMeshData.Vertices.data(), bInDynamic);

    uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
    uint32 IndexNum = GetSize(InMeshData.Indices);
    OutBuffers.IndexBuffer = IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, InMeshData.Indices.data()) : nullptr;

    uint32 InstanceSize = static_cast<uint32>(sizeof(FVertex_Instance));
    FVertex_Instance Temp = {};
    OutBuffers.InstanceBuffer = std::make_shared<MVertexBuffer>(InstanceSize, 1, &Temp, bInDynamic);
}

GraphicDevice::Exception::Exception(const int line, const char *file, const HRESULT hr)
	: EngineException(line, file)
	, m_hResult{ hr }
{
}

const std::string GraphicDevice::Exception::TranslateErrorCode(const HRESULT hr)
{
	char *pBuffer = nullptr;

	DWORD message = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPSTR)&pBuffer, 0, nullptr
	);

	if (message == 0)
	{
		return "Unidentified error code";
	}

	std::string errorString = pBuffer;
	LocalFree(pBuffer);

	return errorString;
}

const char *GraphicDevice::Exception::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code]" << GetErrorCode() << std::endl
		<< "[Description]" << GetErrorString() << std::endl
		<< GetOriginString();

	m_whatBuffer = oss.str();
	return m_whatBuffer.c_str();
}

const char *GraphicDevice::Exception::GetType() const
{
	return "Engine GraphicDevice Exception";
}

const std::string GraphicDevice::Exception::GetErrorString() const
{
	return TranslateErrorCode(m_hResult);
}

const HRESULT GraphicDevice::Exception::GetErrorCode() const
{
	return m_hResult;
}
