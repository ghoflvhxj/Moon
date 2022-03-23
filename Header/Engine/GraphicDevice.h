#pragma once
#ifndef __GRAPHIC_DEVICE_H__

#include "EngineException.h"
#include "Vertex.h"

class ENGINE_DLL GraphicDevice
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
	~GraphicDevice();

	GraphicDevice(const GraphicDevice &ref) = delete;
	GraphicDevice(GraphicDevice &&rRef) = delete;
	GraphicDevice &operator=(const GraphicDevice &ref) = delete;

	//-------------------------------------------------------------------------
private:
	const bool initialize();

public:
	const bool BuildInputLayout();

	const bool Refresh();
	void Release();
	void Begin();
	void End();

private:
	const bool buildRasterizerState();
public:
	ID3D11RasterizerState *getRasterizerState(const Graphic::FillMode eFillMode, const Graphic::CullMode eCullMode);
private:
	std::vector<ID3D11RasterizerState *> rasterizerList;

private:
	const bool buildDepthStencilState();
public:
	ID3D11DepthStencilState *getDepthStencilState(const Graphic::DepthWriteMode eDetphWrite);
private:
	std::vector<ID3D11DepthStencilState *> depthStencilStateList;

private:
	const bool buildBlendState();
public:
	ID3D11BlendState *getBlendState(const Graphic::Blend eBlend);
private:
	std::vector<ID3D11BlendState*> _blendStateList;

public:
	ID3D11Device *getDevice();
private:
	ID3D11Device *m_pDevice;

public:
	ID3D11DeviceContext *getContext();
public:
	ID3D11DeviceContext *getImmediateContext();
	ID3D11DeviceContext *getDefferedContext();
private:
	ID3D11DeviceContext *m_pImmediateContext;	// 즉시 문맥: 싱글 쓰레드용
	ID3D11DeviceContext *m_pDeferredContext;	// 지연 문맥: 멀티 쓰레드용 CreateDeferredContext로 생성한다

private:
	IDXGISwapChain *m_pSwapChain;

private:
	ID3D11RenderTargetView *m_pRenderTargetView;
	ID3D11DepthStencilView *m_pDepthStencilView;
	ID3D11Texture2D *m_pDepthStencilBuffer;
	//---------------------------------------------
private:
	ID3D11InputLayout *m_pInputLayout;

private:
	D3D11_VIEWPORT _viewport;
};

#define FAILED_CHECK_THROW(hr) if((HRESULT)hr < 0) throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError());

#define __GRAPHIC_DEVICE_H__
#endif

