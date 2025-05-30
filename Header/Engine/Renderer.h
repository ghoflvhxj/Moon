#pragma once
#ifndef __RENDERER_H__

#include "Render.h"

class SceneComponent;
class StaticMeshComponent;
class VertexBuffer;
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
	explicit Renderer(void) noexcept;
	~Renderer(void) noexcept;
	void Release();

private:
	void initialize(void) noexcept;
private:
	std::shared_ptr<StaticMeshComponent> _pMeshComponent;

	// ������ Primitive �߰�
public:
	void AddPrimitive(std::shared_ptr<PrimitiveComponent> &pComponent);
	std::map<uint32, std::vector<FPrimitiveData>>& GetDeferredPrimitiveData() { return DeferredPrimitiveDataMap; }
protected:
	std::vector<std::weak_ptr<PrimitiveComponent>> CachedPrimitiveComponents;
	std::vector<std::weak_ptr<PrimitiveComponent>> RenderablePrimitiveComponents;
	std::map<uint32, std::vector<FPrimitiveData>> ForwardPrimitiveDataMap;
	std::map<uint32, std::vector<FPrimitiveData>> DeferredPrimitiveDataMap;
	std::map<uint32, std::vector<std::shared_ptr<VertexBuffer>>> VertexBufferMap;


	// ������Ʈ�� �������� ���� ������ �ڷ����� ������ ������Ʈ�� �������� ���̷��� �ǵ�...
public:
	void addDirectionalLightInfoForShadow(const Vec3 &direction);
private:
	std::vector<Vec3> _directionalLightDirection;

	// ���� Ÿ��
public:
	void addRenderTargetForDebug(const std::wstring name);
	std::shared_ptr<RenderTarget>& GetRenderTarget(ERenderTarget RenderTarget) { return _renderTargets[static_cast<int32>(RenderTarget)]; }
private:
	RenderTargets _renderTargets;
#ifdef _DEBUG
	std::unordered_map<std::wstring, std::shared_ptr<StaticMeshComponent>> _renderTargetMeshs;
#endif

	// ���� �н�
private:
	std::vector<std::shared_ptr<RenderPass>> _renderPasses;

public:
	void render();
	void renderScene();
	void renderText();
private:
	void FrustumCulling();
	// PerConstant, PerTick �ܽ�źƮ ���۸� ������Ʈ��.
	void updateConstantBuffer();
	inline void copyBufferData(std::vector<std::vector<VariableInfo>> &infos, ConstantBuffersLayer layer, uint32 index, const void *pData);
	
public:
	uint32 TotalPrimitiveNum = 0;
	uint32 showPrimitiveCount = 0;
	uint32 culledPrimitiveCount = 0;

public:
	void toggleRenderTarget();
private:
	bool _drawRenderTarget;

public:
	const bool IsDirtyConstant() const;
private:
	bool _bDirtyConstant;

	void Test(std::vector<Mat4>& lightViewProj, std::vector<Vec4>& lightPosition);
private:
	std::vector<float> _cascadeDistance;

public:
	RENDERER_OPTION(DrawCollision)
};

#define __RENDERER_H__
#endif