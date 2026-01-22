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

bool GraphicDevice::bReverseDepth = true;

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

GraphicDevice::~GraphicDevice()
{

}

float GraphicDevice::GetNear()
{
    return bReverseDepth ? 1.f : 0.f;
}

float GraphicDevice::GetFar()
{
    return bReverseDepth ? 0.f : 1.f;
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
    uint32 Flags =0;
#ifdef _DEBUG
    Flags |= (uint32)D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG;
#endif

    FAILED_CHECK_THROW(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        (D3D11_CREATE_DEVICE_FLAG)Flags,
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

    for (int i = 0; i < 3; ++i)
    {
        D3D11_QUERY_DESC QueryDesc = {};
        QueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        QueryDesc.MiscFlags = 0;
        m_pDevice->CreateQuery(&QueryDesc, Query[i].GetAddressOf());

        for (int j = 0; j < 15; ++j)
        {
            QueryDesc.Query = D3D11_QUERY_TIMESTAMP;
            m_pDevice->CreateQuery(&QueryDesc, Start[i][j].GetAddressOf());
            m_pDevice->CreateQuery(&QueryDesc, Finish[i][j].GetAddressOf());
        }
    }
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

    SafeRelease(DepthPrePassRS);

    SafeRelease(m_pInputLayout); 

    SharedBuffers.clear();

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

    Query = {};
    Start = {};
    Finish = {};

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
    if (InRenderTarget->AsRenderTargetView())
    {
        getContext()->ClearRenderTargetView(InRenderTarget->AsRenderTargetView(), reinterpret_cast<const float*>(&InColor));
    }

    if (InRenderTarget->getDepthStencilView())
    {
        /****************************
                    0.f     1.f
         Normal     Near    Far
         Reverse    Far     Near
        ****************************/ 
        getContext()->ClearDepthStencilView(InRenderTarget->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, GetFar(), 0u);
    }
}

ID3D11DepthStencilView* GraphicDevice::GetDepthStencilView()
{
    const FWindowRenderData& Test = WindowRenderDatas[WindowID];
    return Test.DepthStencilView.Get();
}

ID3D11ShaderResourceView* GraphicDevice::GetDepthResourceView()
{
    const FWindowRenderData& Test = WindowRenderDatas[WindowID];
    return Test.DepthSRV.Get();
}

ID3D11ShaderResourceView* GraphicDevice::GetStencilResourceView()
{
    const FWindowRenderData& Test = WindowRenderDatas[WindowID];
    return Test.StencilSRV.Get();
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
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    //DXGI_SWAP_CHAIN_FULLSCREEN_DESC* FullScreenDescPtr = nullptr;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullscreenDesc = {};
    FullscreenDesc.RefreshRate.Numerator = 0;
    FullscreenDesc.RefreshRate.Denominator = 0;
    FullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    //FullscreenDesc.Windowed = Window->IsFullScreen() ? TRUE : FALSE;
    FullscreenDesc.Windowed = TRUE;
    FullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

    //if (Window->IsFullScreen())
    //{
    //    FullScreenDescPtr = &FullscreenDesc;
    //}

    FAILED_CHECK_THROW(factory->CreateSwapChainForHwnd(m_pDevice, InWorldRenderInfo.DstWindow->getHandle(), &swapDesc, &FullscreenDesc, nullptr, NewWindowRenderData.SwapChain.GetAddressOf()));
    NewWindowRenderData.SwapChain->QueryInterface(IID_PPV_ARGS(NewWindowRenderData.SwapChain3.GetAddressOf()));

    // 렌더 타겟 뷰 생성
    std::array<ID3D11Texture2D*, 2> SawpChainBuffers = {};
    for (uint32 i = 0; i < GetSize(SawpChainBuffers); ++i)
    {
        FAILED_CHECK_THROW(NewWindowRenderData.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SawpChainBuffers[i]));

        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        RTVDesc.Texture2D.MipSlice = 0;
        FAILED_CHECK_THROW(m_pDevice->CreateRenderTargetView(SawpChainBuffers[i], &RTVDesc, NewWindowRenderData.RenderTargetViews[i].GetAddressOf()));
    }
    SafeReleaseArray(SawpChainBuffers);

    // 텍스쳐 생성
    D3D11_TEXTURE2D_DESC TexureDesc = { };
    TexureDesc.Width = Window->GetWidth<UINT>();
    TexureDesc.Height = Window->GetHeight<UINT>();
    TexureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    TexureDesc.SampleDesc.Count = 1;
    TexureDesc.SampleDesc.Quality = 0;
    TexureDesc.Usage = D3D11_USAGE_DEFAULT;
    TexureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    TexureDesc.ArraySize = 1;
    TexureDesc.MipLevels = 1;
    TexureDesc.CPUAccessFlags = 0;
    TexureDesc.MiscFlags = 0;
    m_pDevice->CreateTexture2D(&TexureDesc, nullptr, NewWindowRenderData.DepthStencilTexture.GetAddressOf());

    // 깊이 스텐실 뷰 생성
    D3D11_DEPTH_STENCIL_VIEW_DESC ViewDesc = {};
    ViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
    ViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ViewDesc.Texture2D.MipSlice = 0;
    FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilView(NewWindowRenderData.DepthStencilTexture.Get(), &ViewDesc, NewWindowRenderData.DepthStencilView.GetAddressOf()));

    // 깊이 쉐이더 리소스 뷰
    D3D11_SHADER_RESOURCE_VIEW_DESC DepthResourceViewDesc = { };
    DepthResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
    DepthResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    DepthResourceViewDesc.Texture2D.MipLevels = -1;
    DepthResourceViewDesc.Texture2D.MostDetailedMip = 0;
    FAILED_CHECK_THROW(m_pDevice->CreateShaderResourceView(NewWindowRenderData.DepthStencilTexture.Get(), &DepthResourceViewDesc, NewWindowRenderData.DepthSRV.GetAddressOf()));

    // 스텐실 쉐이더 리소스 뷰
    D3D11_SHADER_RESOURCE_VIEW_DESC StencilResourceViewDesc = { };
    StencilResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
    StencilResourceViewDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
    StencilResourceViewDesc.Texture2D.MipLevels = -1;
    StencilResourceViewDesc.Texture2D.MostDetailedMip = 0;
    FAILED_CHECK_THROW(m_pDevice->CreateShaderResourceView(NewWindowRenderData.DepthStencilTexture.Get(), &StencilResourceViewDesc, NewWindowRenderData.StencilSRV.GetAddressOf()));

    WindowRenderDatas[InWorldRenderInfo.DstWindow->GetID()] = NewWindowRenderData;
}

void GraphicDevice::UpdateWindowSize(uint32 InWindowID, uint32 InOldWidth, uint32 InOldHeight, uint32 InNewWidth, uint32 InNewHeight, bool InFullScreen)
{
    std::cout << "Update Window Size" << std::endl;

    GetPostLoopDelegate().Add([&, InWindowID, InNewWidth, InNewHeight, InFullScreen]() {
        //getContext()->ClearState();

        UINT Width = static_cast<UINT>(InNewWidth);
        UINT Height = static_cast<UINT>(InNewHeight);

        auto& WindowRenderData = WindowRenderDatas[InWindowID];

        WindowRenderData.DepthStencilTexture.Reset();
        WindowRenderData.DepthStencilView.Reset();
        WindowRenderData.RenderTargetViews[0].Reset();
        WindowRenderData.RenderTargetViews[1].Reset();

        if (WindowRenderData.SwapChain->SetFullscreenState(InFullScreen ? TRUE : FALSE, NULL) != S_OK)
        {
            std::cout << "SetFullscreenState Failed" << std::endl;
        }

        //WindowRenderData.SwapChain->ResizeTarget()

        HRESULT HR = WindowRenderData.SwapChain->ResizeBuffers(0, InNewWidth, InNewHeight, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (HR == S_OK)
        {
            // 렌더 타겟 뷰 생성
            std::array<ID3D11Texture2D*, 2> SawpChainBuffers = {};
            for (uint32 i = 0; i < GetSize(SawpChainBuffers); ++i)
            {
                FAILED_CHECK_THROW(WindowRenderData.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&SawpChainBuffers[i]));

                D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
                RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                RTVDesc.Texture2D.MipSlice = 0;
                FAILED_CHECK_THROW(m_pDevice->CreateRenderTargetView(SawpChainBuffers[i], &RTVDesc, WindowRenderData.RenderTargetViews[i].GetAddressOf()));
            }
            SafeReleaseArray(SawpChainBuffers);

            // 텍스쳐 생성
            D3D11_TEXTURE2D_DESC TexureDesc = { };
            TexureDesc.Width = Width;
            TexureDesc.Height = Height;
            TexureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            TexureDesc.SampleDesc.Count = 1;
            TexureDesc.SampleDesc.Quality = 0;
            TexureDesc.Usage = D3D11_USAGE_DEFAULT;
            TexureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
            TexureDesc.ArraySize = 1;
            TexureDesc.MipLevels = 1;
            TexureDesc.CPUAccessFlags = 0;
            TexureDesc.MiscFlags = 0;
            m_pDevice->CreateTexture2D(&TexureDesc, nullptr, WindowRenderData.DepthStencilTexture.GetAddressOf());

            // 깊이 스텐실 뷰 생성
            D3D11_DEPTH_STENCIL_VIEW_DESC ViewDesc = {};
            ViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
            ViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            ViewDesc.Texture2D.MipSlice = 0;
            FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilView(WindowRenderData.DepthStencilTexture.Get(), &ViewDesc, WindowRenderData.DepthStencilView.GetAddressOf()));

            D3D11_SHADER_RESOURCE_VIEW_DESC DepthResourceViewDesc = { };
            DepthResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
            DepthResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            DepthResourceViewDesc.Texture2D.MipLevels = -1;
            DepthResourceViewDesc.Texture2D.MostDetailedMip = 0;
            FAILED_CHECK_THROW(m_pDevice->CreateShaderResourceView(WindowRenderData.DepthStencilTexture.Get(), &DepthResourceViewDesc, WindowRenderData.DepthSRV.GetAddressOf()));

            // 스텐실 쉐이더 리소스 뷰
            D3D11_SHADER_RESOURCE_VIEW_DESC StencilResourceViewDesc = { };
            StencilResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
            StencilResourceViewDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
            StencilResourceViewDesc.Texture2D.MipLevels = -1;
            StencilResourceViewDesc.Texture2D.MostDetailedMip = 0;
            FAILED_CHECK_THROW(m_pDevice->CreateShaderResourceView(WindowRenderData.DepthStencilTexture.Get(), &StencilResourceViewDesc, WindowRenderData.StencilSRV.GetAddressOf()));
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
     g_pGraphicDevice->getContext()->IASetInputLayout(g_pGraphicDevice->GetInputLayout());

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

    SetToDefault();

    WindowID = InWindowID;

    if (Width != InWidth || Height != InHeight)
    {
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

    getContext()->Begin(Query[Counter].Get());
}

void GraphicDevice::End()
{
    if (WindowID == -1)
    {
        return;
    }

    Test();

    const FWindowRenderData& Test = WindowRenderDatas[WindowID];
    Test.SwapChain3->Present(0u, 0u);

    UINT BufferIndex = Test.SwapChain3->GetCurrentBackBufferIndex();
    getContext()->ClearRenderTargetView(Test.RenderTargetViews[BufferIndex].Get(), reinterpret_cast<const float*>(&EngineColors::Blue));
    getContext()->ClearDepthStencilView(Test.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, GetFar(), 0u);

    getContext()->End(Query[Counter].Get());

    int readIndex = (Counter + 1) % 3;

    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT DisjointData = {};
    if (getContext()->GetData(Query[readIndex].Get(), &DisjointData, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0) == S_OK)
    {
        RenderPassTimes.clear();
        for (int RenderPassIndex = 0; RenderPassIndex < 15; RenderPassIndex++)
        {
            UINT64 StartTime = 0, FinishTime = 0;
            getContext()->GetData(Start[readIndex][RenderPassIndex].Get(), &StartTime, sizeof(UINT64), 0);
            getContext()->GetData(Finish[readIndex][RenderPassIndex].Get(), &FinishTime, sizeof(UINT64), 0);

            std::wostringstream sout;
            sout << "RenderPass" << RenderPassIndex << " Time: " << (FinishTime - StartTime) / (float)DisjointData.Frequency * 1000.f << endl;

            RenderPassTimes.push_back(sout.str());
        }
    }

    ++Counter;
    Counter %= 3;
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
        //rd.DepthBias = 100000;
        //rd.SlopeScaledDepthBias = 1.f;
        FAILED_CHECK_THROW(m_pDevice->CreateRasterizerState(&rd, &DepthPrePassRS));
        rd.DepthBias = 0;
        rd.SlopeScaledDepthBias = 0.f;
    }

    {
        //rd.DepthBias = 100000;
        //rd.SlopeScaledDepthBias = 1.f;
        FAILED_CHECK_THROW(m_pDevice->CreateRasterizerState(&rd, ShadowDepthRS.GetAddressOf()));
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
        return DepthPrePassRS;
    }

	return RasterizeStates[(EnumToIndex(eFillMode) * CastValue<uint32>(Graphic::CullMode::Count)) + EnumToIndex(eCullMode)];
}

bool GraphicDevice::buildDepthStencilState()
{
    /*********************************************
                Cur         Buffer      기록
     Normal     0.1f        0.5f        O
     Reverse    0.1f        0.5f        X
     Normal     0.5f        0.1f        X
     Reverse    0.5f        0.1f        O
    *********************************************/


	ID3D11DepthStencilState *pDepthStencilState = nullptr;
	//-------------------------------------------------------------------------------------
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = bReverseDepth ? D3D11_COMPARISON_GREATER : D3D11_COMPARISON_LESS;
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

    {
        D3D11_DEPTH_STENCIL_DESC Copy = dsd;
        Copy.DepthFunc = D3D11_COMPARISON_GREATER;
        Copy.StencilEnable = FALSE;
        FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&Copy, LinearDepthStencilState.GetAddressOf()));
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
    /**********************************************************
     Index      DpethCompare    
       0            X
    **********************************************************/
	auto CreateSamplerLambda = [this](D3D11_SAMPLER_DESC& BaseSamplerDesc)
	{
		ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK(m_pDevice->CreateSamplerState(&BaseSamplerDesc, &pSamplerState));
		SamplerStates.emplace_back(pSamplerState);
	};

	D3D11_SAMPLER_DESC BaseSamplerDesc = {};
	BaseSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	BaseSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	BaseSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	BaseSamplerDesc.BorderColor[0] = 1.f;
	BaseSamplerDesc.BorderColor[1] = 1.f;
	BaseSamplerDesc.BorderColor[2] = 1.f;
	BaseSamplerDesc.BorderColor[3] = 1.f;
	BaseSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	BaseSamplerDesc.MaxAnisotropy = 1u;
	BaseSamplerDesc.MinLOD = 0.f;
	BaseSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	BaseSamplerDesc.MipLODBias = 0.f;

	// Point
    {
        D3D11_SAMPLER_DESC SamplerDesc = BaseSamplerDesc;
        SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
        CreateSamplerLambda(SamplerDesc);
    }

	// Linear
    {
        D3D11_SAMPLER_DESC SamplerDesc = BaseSamplerDesc;
        SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
        SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
        SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
        SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        CreateSamplerLambda(SamplerDesc);
    }

	// Anisotropic
    {
        D3D11_SAMPLER_DESC SamplerDesc = BaseSamplerDesc;
        SamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
        SamplerDesc.MaxAnisotropy = 1u;
        CreateSamplerLambda(SamplerDesc);
    }

	// Comparison MIN MAG LINEAR MIP POINT
    {
        BaseSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        BaseSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        BaseSamplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        BaseSamplerDesc.BorderColor[0] = 1.f;
        BaseSamplerDesc.MaxAnisotropy = 1u;
        BaseSamplerDesc.MipLODBias = 0.f;
    }

    // 가까운 거
    {
        D3D11_SAMPLER_DESC SamplerDesc = BaseSamplerDesc;
        SamplerDesc.ComparisonFunc = bReverseDepth ? D3D11_COMPARISON_GREATER : D3D11_COMPARISON_LESS;
        ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState), false);
        SamplerStates.emplace_back(pSamplerState);
        m_pImmediateContext->PSSetSamplers(2, 1, &pSamplerState);
    }

    // 먼 거
    {
        D3D11_SAMPLER_DESC SamplerDesc = BaseSamplerDesc;
        SamplerDesc.ComparisonFunc = bReverseDepth ? D3D11_COMPARISON_LESS : D3D11_COMPARISON_GREATER;
        ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState), false);
        SamplerStates.emplace_back(pSamplerState);
        m_pImmediateContext->PSSetSamplers(3, 1, &pSamplerState);
    }

    // 작은 거
    {
        D3D11_SAMPLER_DESC SamplerDesc = BaseSamplerDesc;
        SamplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
        ID3D11SamplerState* pSamplerState = nullptr;
        FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&SamplerDesc, &pSamplerState), false);
        SamplerStates.emplace_back(pSamplerState);
        m_pImmediateContext->PSSetSamplers(4, 1, &pSamplerState);
    }

    m_pImmediateContext->PSSetSamplers(0, 1, &SamplerStates[0]);
    m_pImmediateContext->PSSetSamplers(1, 1, &SamplerStates[1]);

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
	_spriteFont = std::make_unique<SpriteFont>(m_pDevice, TEXT("MyFont.spritefont"));
	_spriteBatch = std::make_unique<SpriteBatch>(m_pImmediateContext);

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

void GraphicDevice::LinearDepthStencil()
{
   getContext()->OMSetDepthStencilState(LinearDepthStencilState.Get(), 0);
   getContext()->OMSetBlendState(getBlendState(Graphic::Blend::Object), nullptr, 0xffffffff);
}

void GraphicDevice::PSSetSRV(UINT InSlot, uint32 InSRVID)
{
    ID3D11ShaderResourceView* RawSRV = StructuredBufferSRVs[InSRVID].Get();
    getContext()->PSSetShaderResources(InSlot, 1, &RawSRV);
}

void GraphicDevice::QueryStart(uint32 InIndex)
{
    getContext()->End(Start[Counter][InIndex].Get());
}

void GraphicDevice::QueryFinish(uint32 InIndex)
{
    getContext()->End(Finish[Counter][InIndex].Get());
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

ComPtr<ID3D11ShaderResourceView> GraphicDevice::CreateStructuredBufferSRV(ID3D11Buffer* InBuffer, UINT InElementNum, UINT InElementSize)
{
    // 일단 StructuredBuffer 용으로만 만들어 놓음
    D3D11_BUFFER_SRV BufferSRV = {};
    BufferSRV.FirstElement = 0;
    BufferSRV.ElementOffset = 0;
    BufferSRV.ElementWidth = InElementSize;
    BufferSRV.NumElements = InElementNum;

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Buffer = BufferSRV;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

    ComPtr<ID3D11ShaderResourceView> ShaderResourceView = nullptr;
    g_pGraphicDevice->getDevice()->CreateShaderResourceView(InBuffer, &desc, &ShaderResourceView);

    return ShaderResourceView;
}

MStructuredBuffer GraphicDevice::AddStructuredBuffer(const void* InData, UINT InDataSize, UINT InElementNum, UINT InElementSize)
{
    ComPtr<ID3D11Buffer> RawBuffer = CreateStructuredBuffer(InData, InDataSize, InElementNum, InElementSize);
    if (RawBuffer == nullptr)
    {
        return MStructuredBuffer();
    }

    ComPtr<ID3D11ShaderResourceView> RawSRV = CreateStructuredBufferSRV(RawBuffer.Get(), InElementNum, InElementSize);
    if (RawSRV == nullptr)
    {
        return MStructuredBuffer();
    }

    StructuredBuffers[BufferCounter] = RawBuffer;
    StructuredBufferSRVs[SRVCounter] = RawSRV;

    MStructuredBuffer NewBuffer;
    NewBuffer.Init(InElementSize, InElementNum);
    NewBuffer.SetBufferID(BufferCounter);
    NewBuffer.SetSRVID(SRVCounter);

    ++BufferCounter;
    ++SRVCounter;

    return NewBuffer;
}

ComPtr<ID3D11Buffer> GraphicDevice::CreateStructuredBuffer(const void* InData, UINT InDataSize, UINT InElementNum, UINT InElementSize)
{
    // 일단 StructuredBuffer 용으로만 만들어 놓음
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.ByteWidth = InDataSize;
    BufferDesc.StructureByteStride = InElementSize;

    D3D11_SUBRESOURCE_DATA SubDesc = {};
    SubDesc.pSysMem = InData;

    ComPtr<ID3D11Buffer> NewRawBuffer = nullptr;
    FAILED_CHECK_THROW(m_pDevice->CreateBuffer(&BufferDesc, &SubDesc, NewRawBuffer.GetAddressOf()));

    return NewRawBuffer;
}

void GraphicDevice::UpdateStructuredBuffer(MStructuredBuffer& InBuffer, const void* InData, UINT InDataSize, UINT InElementNum, UINT InElementSize)
{
    bool bShouldCreate = InBuffer.GetBufferSize() != InDataSize;

    if (bShouldCreate)
    {
        ComPtr<ID3D11Buffer> NewRawBuffer = CreateStructuredBuffer(InData, InDataSize, InElementNum, InElementSize);
        ComPtr<ID3D11ShaderResourceView> NewRawSRV = CreateStructuredBufferSRV(NewRawBuffer.Get(), InElementNum, InElementSize);

#ifdef _DEBUG
        std::wstring Result;
        if (!NewRawBuffer)
        {
            Result += TEXT("ID3D11Buffer 생성 실패");
        }

        if (!NewRawSRV)
        {
            Result += TEXT("ID3D11ShaderResourceView 생성 실패");
        }

        if (!Result.empty())
        {
            return;
        }
#endif

        StructuredBuffers[InBuffer.GetBufferID()] = NewRawBuffer;
        StructuredBufferSRVs[InBuffer.GetSRVID()] = NewRawSRV;
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE MappedSubResource = {};
        getContext()->Map(GetRawBuffer(InBuffer), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &MappedSubResource);
        memcpy(MappedSubResource.pData, InData, InDataSize);
        getContext()->Unmap(GetRawBuffer(InBuffer), 0u);
    }
}

ID3D11Buffer* GraphicDevice::GetRawBuffer(MStructuredBuffer& InBuffer)
{
    return StructuredBuffers[InBuffer.GetBufferID()].Get();
}

void GraphicDevice::GetBuffers(FMeshBufferContainer& OutBuffers, const std::shared_ptr<MMesh>& InMesh)
{
    const std::wstring& AssetPath = InMesh->GetAssetPath();
    GetBuffers(OutBuffers, AssetPath);
}

void GraphicDevice::GetBuffers(FMeshBufferContainer& OutBuffers, const std::wstring InKey)
{
    auto& Iter = SharedBuffers.find(InKey);

    if (SharedBuffers.end() == Iter)
    {
        return;
    }

    OutBuffers = Iter->second;
}

void GraphicDevice::GetPrivateBuffers(FMeshBufferContainer& OutBuffers, uint32 InPID)
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
    FMeshBuffers NewSharedBuffers = {};
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

        FMeshBuffers NewSharedBuffers = {};
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

        FMeshBuffers NewPrivateBuffers = {};
        MakeBuffer(NewPrivateBuffers, MeshData, true);
        PrivateBuffers[InPID].AddBuffers(i, NewPrivateBuffers);
    }
}

void GraphicDevice::MakeBuffer(FMeshBuffers& OutBuffers, const FMeshData& InMeshData, bool bInDynamic)
{
    uint32 VertexSize = CastValue<uint32>(sizeof(Vertex));
    uint32 VertexNum = GetSize(InMeshData.Vertices);
    OutBuffers.VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, VertexNum, InMeshData.Vertices.data(), bInDynamic);

    uint32 IndexSize = CastValue<uint32>(sizeof(uint32));
    uint32 IndexNum = GetSize(InMeshData.Indices);
    OutBuffers.IndexBuffer = IndexNum > 0 ? std::make_shared<MIndexBuffer>(IndexSize, IndexNum, InMeshData.Indices.data()) : nullptr;

    uint32 InstanceSize = static_cast<uint32>(sizeof(FVertex_Instance));
    FVertex_Instance Temp = {};
    OutBuffers.InstanceBuffer = std::make_shared<MVertexBuffer>(InstanceSize, 1, &Temp, true);
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
