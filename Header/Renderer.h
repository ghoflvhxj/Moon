#pragma once
#ifndef __RENDERER_H__

class SceneRenderer;
class CollisionRenderer;

class PrimitiveComponent;
class MeshComponent;
class LightComponent;
class SkyComponent;
class CollisionShapeComponent;

class RenderTarget;
class RenderPass;

class ENGINE_DLL Renderer 
{
	enum class RenderType
	{
		Scene, Collision
	};

public:
	explicit Renderer(void) noexcept;
	~Renderer(void) noexcept;
private:
	void initialize(void) noexcept;
private:
	std::shared_ptr<MeshComponent> _pMeshComponent;

	// ·»´õ Å¸°Ù
public:
	const bool addRenderTarget(const std::wstring name);
private:
	std::unordered_map<std::wstring, std::shared_ptr<RenderTarget>> _renderTargetMap;
#ifdef _DEBUG
	std::unordered_map<std::wstring, std::shared_ptr<MeshComponent>> _renderTargetMeshMap;
#endif

	// ·»´õ ÆÐ½º
public:
	const bool addRenderPass(const std::wstring name);
private:
	std::unordered_map<std::wstring, std::shared_ptr<RenderPass>> _renderPassMap;

public:
	void render(void);

public:
	void addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> pComponent) noexcept;
	void addLightComponent(std::shared_ptr<LightComponent> pComponent) noexcept;
	void addSkyComponent(std::shared_ptr<SkyComponent> pComponent) noexcept;
	void addCollisionShapeComponent(std::shared_ptr<PrimitiveComponent> pComponent) noexcept;
private:
	std::shared_ptr<SceneRenderer> _pSceneRenderer;
	std::shared_ptr<CollisionRenderer> _pCollisionRenderer;

public:
	void toggleRenderTarget();
private:
	bool _drawRenderTarget;

};

#define __RENDERER_H__
#endif