#pragma once
#ifndef __RENDERER_H__

#include "Render.h"

class LightComponent;
class StaticMeshComponent;

class StaticMesh;

class ENGINE_DLL Renderer 
{
public:
	template <class T>
	static std::shared_ptr<T> CreateRenderPass()
	{
		return std::make_shared<T>();
	}

public:
	explicit Renderer(void) noexcept;
	~Renderer(void) noexcept;
	void Release();

private:
	void initialize(void) noexcept;
private:
	std::shared_ptr<StaticMeshComponent> _pMeshComponent;

	// ∑ª¥ı ø¿∫Í¡ß∆Æ
public:
	void addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> pComponent);
private:
	std::vector<std::shared_ptr<PrimitiveComponent>> _primitiveComponents;

	// ∑ª¥ı ≈∏∞Ÿ
public:
	void addRenderTargetForDebug(const std::wstring name);
private:
	RenderTargets _renderTargets;
#ifdef _DEBUG
	std::unordered_map<std::wstring, std::shared_ptr<StaticMeshComponent>> _renderTargetMeshs;
#endif

	// ∑ª¥ı ∆–Ω∫
private:
	std::vector<std::shared_ptr<RenderPass>> _renderPasses;

public:
	void render(void);
private:
	void updateConstantBuffer();
	inline void copyBufferData(std::vector<std::vector<VariableInfo>> &infos, ConstantBuffersLayer layer, uint32 index, const void *pData);

public:
	void toggleRenderTarget();
private:
	bool _drawRenderTarget;

private:
	void renderMesh();
	void test(PrimitiveData &renderData);

public:
	const bool IsDirtyConstant() const;
private:
	bool _bDirtyConstant;
};

#define __RENDERER_H__
#endif