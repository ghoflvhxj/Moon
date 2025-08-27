#pragma once

#include "Include.h"
#include "Module/Module.h"

#include <wrl/client.h>

// DirectXTK
#include "DirectXTK/SpriteFont.h"

#include "EngineException.h"
#include "Vertex.h"

class VertexShader;
class PixelShader;
class MGeometryShader;
#include "ShaderManager.h"


using Microsoft::WRL::ComPtr;

enum class ESamplerFilter
{
	Point,
	Linear,
	Anisotropic,
	Count
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
    void Begin();
    void End();
    bool Refresh();

	bool BuildInputLayout();

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
	ID3D11RasterizerState *getRasterizerState(const Graphic::FillMode eFillMode, const Graphic::CullMode eCullMode);
	ID3D11DepthStencilState *getDepthStencilState(const uint32 InFlag);
	ID3D11BlendState *getBlendState(const Graphic::Blend eBlend);
private:
    bool buildSamplerState();
    bool buildRasterizerState();
    bool buildDepthStencilState();
    bool buildBlendState();

private:
	std::vector<ID3D11SamplerState*>		Samplers;
	std::vector<ID3D11RasterizerState*>		_rasterizerList;
	std::map<uint32, ID3D11DepthStencilState*>	_depthStencilStateList;
	std::vector<ID3D11BlendState*>			_blendStateList;

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
	ID3D11DeviceContext *getContext();
public:
	ID3D11DeviceContext *getImmediateContext();
	ID3D11DeviceContext *getDefferedContext();
private:
	ID3D11DeviceContext *m_pImmediateContext;	// 즉시 문맥: 싱글 쓰레드용
	ID3D11DeviceContext *m_pDeferredContext;	// 지연 문맥: 멀티 쓰레드용 CreateDeferredContext로 생성한다

private:
    ComPtr<IDXGISwapChain1> m_pSwapChain;
	//IDXGISwapChain1 *m_pSwapChain;

public:
    ID3D11RenderTargetView* m_pRenderTargetView;
    ID3D11RenderTargetView* m_pRenderTargetView2;
	ID3D11DepthStencilView *m_pDepthStencilView;
	ID3D11Texture2D *m_pDepthStencilBuffer;
	//---------------------------------------------
public:
	ID3D11InputLayout *m_pInputLayout;

public:
    // 피직스 테스트용
    ID3D11InputLayout* SimpleLayout = nullptr;

private:
	D3D11_VIEWPORT _viewport;

    REFLECT(GraphicDevice)
};

#define FAILED_CHECK_THROW(hr) if((HRESULT)hr < 0) throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError());
#define FAILED_CHECK_THROW_MSG(hr, message) if((HRESULT)hr < 0) { assert(false && TEXT(message)); throw GraphicDevice::Exception(__LINE__, __FILE__, GetLastError()); }

