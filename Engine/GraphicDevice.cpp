#include "Include.h"
#include "GraphicDevice.h"

#include "Vertex.h"
#include "InputLayout.h"

#include "ShaderManager.h"
#include "ShaderLoader.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "MainGameSetting.h"

#include <dxgidebug.h>
#include <dxgi1_3.h>

#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

GraphicDevice::GraphicDevice()
	: m_pDevice{ nullptr }
	, m_pImmediateContext{ nullptr }
	, m_pDeferredContext{ nullptr }
	, m_pSwapChain{ nullptr }
	, m_pRenderTargetView{ nullptr }
	, m_pDepthStencilView{ nullptr }
	, m_pDepthStencilBuffer{ nullptr }
	, _spriteBatch{ nullptr }
	, _spriteFont{ nullptr }

	, m_pInputLayout{ nullptr }
	//, m_pPixelShader{ nullptr }
	//, m_pVertexShader{ nullptr }
	, _viewport{ }
{
	initializeGrahpicDevice();

	buildSamplerState();
	buildRasterizerState();
	buildDepthStencilState();
	buildBlendState();

	initializeDirectXTK();
}

GraphicDevice::~GraphicDevice()
{
	/*----------------------------------
	* 전역 정적 객체들의 생성과 소멸에 순서가 없어서, 자원들을 소멸자에 해제하지 않고 Relase함수로 빼낸다.
	* 윈도우가 먼저 소멸될 때는 스왑체인이 해제가 안되기 때문! 
	----------------------------------*/

	//m_pPixelShader->Release();
	//m_pVertexShader->Release();

	//m_pInputLayout->Release();

	//m_pDepthStencilView->Release();
	//m_pRenderTargetView->Release();

	//m_pImmediateContext->Release();
	//m_pDevice->Release();

	OutputDebugString(TEXT("--------------------------------------------------"));
}

const bool GraphicDevice::initializeGrahpicDevice()
{
	// 장치, 스왑체인 생성
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = g_pSetting->getResolutionWidth();
	swapDesc.BufferDesc.Height = g_pSetting->getResolutionHeight();
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.OutputWindow = g_hWnd;
	swapDesc.Windowed = TRUE;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Flags = 0;

	FAILED_CHECK_THROW(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&swapDesc, &m_pSwapChain,
		&m_pDevice, nullptr, &m_pImmediateContext
	));	

	// 렌더 타겟 뷰 생성
	ID3D11Texture2D *pBackBuffer = nullptr;
	FAILED_CHECK_THROW(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBuffer));
	FAILED_CHECK_THROW(m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView));
	SafeRelease(pBackBuffer);

	// 깊이 스텐실 뷰 생성
	D3D11_TEXTURE2D_DESC depthStencilDesc = { };
	depthStencilDesc.Width = g_pSetting->getResolutionWidth();
	depthStencilDesc.Height = g_pSetting->getResolutionHeight();
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, nullptr, &m_pDepthStencilView));

	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// 뷰포트
	_viewport.TopLeftX = 0;
	_viewport.TopLeftY = 0;
	_viewport.Width = static_cast<FLOAT>(g_pSetting->getResolutionWidth());
	_viewport.Height = static_cast<FLOAT>(g_pSetting->getResolutionHeight());
	_viewport.MaxDepth = 1.f;
	_viewport.MinDepth = 0.f;

	m_pImmediateContext->RSSetViewports(1, &_viewport);

	return true;
}

const bool GraphicDevice::BuildInputLayout()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescList;
	Vertex::getDesc(inputDescList);

    ID3DBlob *pBlob = g_pShaderManager->getVertexShaderBlob(TEXT("TexVertexShader.cso"));
	
	UINT elementCount = static_cast<UINT>(inputDescList.size());
	FAILED_CHECK_THROW(m_pDevice->CreateInputLayout(&inputDescList[0], elementCount, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout));

	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	return true;
}

const bool GraphicDevice::Refresh()
{
	assert(m_pDevice);
	assert(m_pImmediateContext);
	assert(m_pSwapChain);

	return true;
}

void GraphicDevice::Release()
{
	_spriteFont.reset();
	_spriteBatch.reset();

	SafeReleaseArray(_samplerList);
	SafeReleaseArray(_rasterizerList);
	SafeReleaseArray(_depthStencilStateList);
	SafeReleaseArray(_blendStateList);

	SafeRelease(m_pInputLayout);
	SafeRelease(m_pDepthStencilView);
	SafeRelease(m_pDepthStencilBuffer);
	SafeRelease(m_pRenderTargetView);

	SafeRelease(m_pSwapChain);

	//m_pDeferredContext->ClearState();
	//m_pDeferredContext->Flush();
	//SafeRelease(m_pDeferredContext);
	if (m_pImmediateContext)
	{
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Flush();
	}
	SafeRelease(m_pImmediateContext);

	SafeRelease(m_pDevice);

	IDXGIDebug1* debug = nullptr;
	//g_pGraphicDevice->getDevice()->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug));
	OutputDebugStringW(TEXT("----------------ReportLiveObjectsBegin-------------------\r\n"));
	debug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL);
	OutputDebugStringW(TEXT("----------------ReportLiveObjectsEnd-------------------\r\n"));
	SafeRelease(debug);
}

void GraphicDevice::Begin()
{
	getContext()->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float *>(&EngineColors::Blue));
	getContext()->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
}

const bool GraphicDevice::buildRasterizerState()
{
	_rasterizerList.reserve(CastValue<uint32>(Graphic::FillMode::Count) + CastValue<uint32>(Graphic::CullMode::Count));
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
	rd.ScissorEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;

	//-------------------------------------------------------------------------------------
	for (uint32 fillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME; fillMode <= D3D11_FILL_MODE::D3D11_FILL_SOLID; ++fillMode)
	{
		for (uint32 cullMode = D3D11_CULL_MODE::D3D11_CULL_NONE; cullMode <= D3D11_CULL_MODE::D3D11_CULL_BACK; ++cullMode)
		{
			rd.FillMode = D3D11_FILL_MODE(fillMode);
			rd.CullMode = D3D11_CULL_MODE(cullMode);

			FAILED_CHECK_THROW(m_pDevice->CreateRasterizerState(&rd, &pRasterizerState));
			_rasterizerList.push_back(pRasterizerState);
		}
	}

	return true;
}

ID3D11RasterizerState *GraphicDevice::getRasterizerState(const Graphic::FillMode eFillMode, const Graphic::CullMode eCullMode)
{
	return _rasterizerList[(enumToIndex(eFillMode) * CastValue<uint32>(Graphic::CullMode::Count)) + enumToIndex(eCullMode)];
}

const bool GraphicDevice::buildDepthStencilState()
{
	_depthStencilStateList.reserve(CastValue<uint32>(Graphic::DepthWriteMode::Count));
	ID3D11DepthStencilState *pDepthStencilState = nullptr;

	//-------------------------------------------------------------------------------------
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;
	dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace = dsd.FrontFace;

	//-------------------------------------------------------------------------------------
	FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&dsd, &pDepthStencilState));
	_depthStencilStateList.push_back(pDepthStencilState);

	//-------------------------------------------------------------------------------------
	dsd.DepthEnable = FALSE;

	FAILED_CHECK_THROW(m_pDevice->CreateDepthStencilState(&dsd, &pDepthStencilState));
	_depthStencilStateList.push_back(pDepthStencilState);

	return true;
}

ID3D11DepthStencilState *GraphicDevice::getDepthStencilState(const Graphic::DepthWriteMode eDetphWrite)
{
	return _depthStencilStateList[enumToIndex(eDetphWrite)];
}

const bool GraphicDevice::buildBlendState()
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
	_blendStateList.push_back(pBlendState);

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
	_blendStateList.push_back(pBlendState);

	return true;
}

ID3D11BlendState *GraphicDevice::getBlendState(const Graphic::Blend eBlend)
{
	return _blendStateList[enumToIndex(eBlend)];
}

void GraphicDevice::End()
{
	m_pSwapChain->Present(0u, 0u);
}

const bool GraphicDevice::buildSamplerState()
{
	// 샘플러 스태이트
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
	samplerDesc.BorderColor[0] = 1.f;
	samplerDesc.BorderColor[1] = 1.f;
	samplerDesc.BorderColor[2] = 1.f;
	samplerDesc.BorderColor[3] = 1.f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 16u;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MipLODBias = 0.f;

	ID3D11SamplerState *pSamplerState = nullptr;
	FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&samplerDesc, &pSamplerState), false);
	_samplerList.emplace_back(pSamplerState);
	m_pImmediateContext->PSSetSamplers(0, 1, &pSamplerState);

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.BorderColor[0] = 1.f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
	samplerDesc.MaxAnisotropy = 16u;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MipLODBias = 0.f;

	pSamplerState = nullptr;
	FAILED_CHECK_RETURN(m_pDevice->CreateSamplerState(&samplerDesc, &pSamplerState), false);
	_samplerList.emplace_back(pSamplerState);
	m_pImmediateContext->PSSetSamplers(1, 1, &pSamplerState);

	//m_pImmediateContext->PSSetSamplers(0, CastValue<UINT>(_samplerList.size()), &_samplerList[0]);

	return true;
}

ID3D11SamplerState* GraphicDevice::getSamplerState()
{
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
