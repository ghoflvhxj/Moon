#pragma once
#ifndef __RENDERER_H__

#include "Render.h"

class SceneComponent;
class StaticMeshComponent;
class MVertexBuffer;
class RenderTarget;

class ENGINE_DLL Renderer 
{
public:
	template <class T>
	static std::shared_ptr<T> CreateRenderPass()
	{
		return std::make_shared<T>();
	}

public:
	explicit Renderer() noexcept;
	~Renderer() noexcept;
	void Release();

private:
	void initialize() noexcept;
private:
	std::shared_ptr<StaticMeshComponent> ViewMeshComponent;

	// 렌더할 Primitive 추가
public:
	void AddPrimitive(std::shared_ptr<PrimitiveComponent> &pComponent);
	std::map<uint32, std::vector<FPrimitiveData>>& GetDeferredPrimitiveData() { return DeferredPrimitiveDataMap; }
protected:
	std::vector<std::weak_ptr<PrimitiveComponent>> CachedPrimitiveComponents;
	std::vector<std::weak_ptr<PrimitiveComponent>> RenderablePrimitiveComponents;
	std::map<uint32, std::vector<FPrimitiveData>> ForwardPrimitiveDataMap;
	std::map<uint32, std::vector<FPrimitiveData>> DeferredPrimitiveDataMap;
	std::map<uint32, std::vector<std::shared_ptr<MVertexBuffer>>> VertexBufferMap;

	// 렌더 타겟
public:
	void addRenderTargetForDebug(const std::wstring name);
	std::shared_ptr<RenderTarget>& GetRenderTarget(ERenderTarget RenderTarget) { return _renderTargets[static_cast<int32>(RenderTarget)]; }
private:
	RenderTargets _renderTargets;
#ifdef _DEBUG
	std::unordered_map<std::wstring, std::shared_ptr<StaticMeshComponent>> _renderTargetMeshs;
#endif

	// 렌더 패스
private:
	std::vector<std::shared_ptr<RenderPass>> RenderPasses;

public:
	void Render();
	void RenderScene();
	void RenderText();
private:
	void FrustumCulling();
	void UpdateGlobalConstantBuffer(std::shared_ptr<MShader>& Shader);
    void UpdateTickConstantBuffer(std::shared_ptr<MShader>& Shader);

public:
	uint32 TotalPrimitiveNum = 0;
	uint32 showPrimitiveCount = 0;
	uint32 culledPrimitiveCount = 0;

public:
	void toggleRenderTarget();
private:
	bool _drawRenderTarget;

public:
	const bool IsGlobalBufferDirty() const;
private:
	// 한 프레임 동안에 ConstantBuffer가 변경되었는지 여부를 판단하기 위한 변수
	bool _bDirtyConstant;


public:
    std::vector<FPrimitiveData> PointLightPrimitive;
    std::vector<FPrimitiveData> DirectionalLightPrimitive;
protected:
    std::vector<float> _cascadeDistance;
    std::vector<Vec3> LightPosition;
    std::vector<Mat4> LightViewProj;

    std::vector<Vec3> PointLightPosition;
    std::vector<Mat4> PointLightViewProj;

public:
	RENDERER_OPTION(DrawCollision)
};

#define __RENDERER_H__
#endif