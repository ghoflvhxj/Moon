#pragma once
#ifndef __GRAPHIC_DEVICE_H__

// DirectXTK
#include <SpriteFont.h>

#include "EngineException.h"
#include "Vertex.h"

class VertexShader;
class PixelShader;

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
	const bool initializeGrahpicDevice();

public:
	const bool BuildInputLayout();

	const bool Refresh();
	void Release();
	void Begin();
	void End();

	//-------------------------------------------------------------------------
	// State
public:
	ID3D11SamplerState* getSamplerState();
	ID3D11RasterizerState *getRasterizerState(const Graphic::FillMode eFillMode, const Graphic::CullMode eCullMode);
	ID3D11DepthStencilState *getDepthStencilState(const Graphic::DepthWriteMode eDetphWrite);
	ID3D11BlendState *getBlendState(const Graphic::Blend eBlend);
private:
	const bool buildSamplerState();
	const bool buildRasterizerState();
	const bool buildDepthStencilState();
	const bool buildBlendState();

private:
	std::vector<ID3D11SamplerState*>		_samplerList;
	std::vector<ID3D11RasterizerState*>		_rasterizerList;
	std::vector<ID3D11DepthStencilState *>	_depthStencilStateList;
	std::vector<ID3D11BlendState*>			_blendStateList;

private:
	const bool initializeDirectXTK();
public:
	std::unique_ptr<DirectX::SpriteBatch> _spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> _spriteFont;

	//-------------------------------------------------------------------------
	// �������̽� ����
public:
	void SetVertexShader(std::shared_ptr<VertexShader> &vertexShader);
	void SetPixelShader(std::shared_ptr<PixelShader> &pixelShader);

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
	ID3D11DeviceContext *m_pImmediateContext;	// ��� ����: �̱� �������
	ID3D11DeviceContext *m_pDeferredContext;	// ���� ����: ��Ƽ ������� CreateDeferredContext�� �����Ѵ�

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
#define FAILED_CHECK_THROW_MSG(hr, message) if((HRESULT)hr < 0) { assert(false && TEXT(message)); throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError()); }

#define __GRAPHIC_DEVICE_H__
#endif

