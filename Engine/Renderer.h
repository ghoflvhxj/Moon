#pragma once
#ifndef __RENDERER_H__

#include "Render.h"

class SceneComponent;
class StaticMeshComponent;

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
	void addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> &pComponent);
private:
	std::vector<std::shared_ptr<PrimitiveComponent>> _primitiveComponents;

	// ������Ʈ�� �������� ���� ������ �ڷ����� ������ ������Ʈ�� �������� ���̷��� �ǵ�...
public:
	void addDirectionalLightInfoForShadow(const Vec3 &translation, const Vec3 &rotation);
private:
	std::vector<Vec3> _directionalLightTranslations;
	std::vector<Vec3> _directionalLightForwards;

	// ���� Ÿ��
public:
	void addRenderTargetForDebug(const std::wstring name);
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
	uint32 totalPrimitiveCount = 0;
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
};

#define __RENDERER_H__
#endif