#pragma once
#ifndef __SCENE_RENDERER_H__

class PrimitiveComponent;
class LightComponent;
class SkyComponent;
class CollisionShapeComponent;

class MeshComponent;

class RenderTarget;

class ENGINE_DLL SceneRenderer
{
public:
	using PrimitiveComponentList	= std::vector<std::shared_ptr<PrimitiveComponent>>;
	using LightComponentList		= std::vector<std::shared_ptr<LightComponent>>;
	using SkyComponentList			= std::vector<std::shared_ptr<SkyComponent>>;
	using CollisionComponentList	= std::vector<std::shared_ptr<CollisionShapeComponent>>;
	using CommandLists = std::vector<ID3D11CommandList*>;

public:
	explicit SceneRenderer(std::shared_ptr<MeshComponent> pDeferredRenderMesh);
	~SceneRenderer();
private:
	void initializeRenderTarget();

public:
	void render();
public:
	void renderRenderTargets(void) noexcept;

private:
	void renderObject();
private:
	void fowardRender();
	void deferredRender();
private:
	void renderAlbedo();
	void renderLight();

private:
	void renderSky();
	void renderCollision();

	// 멀티 스레딩용
private:
	static unsigned int RenderFunc(void *pParam);
private:
	CommandLists _commandLists;


public:
	void addPrimitiveComponent(std::shared_ptr<PrimitiveComponent> pComponent);
private:
	PrimitiveComponentList _primitiveComponentList;

public:
	void addLightComponent(std::shared_ptr<LightComponent> pComponent);
private:
	LightComponentList _lightComponentList;

public:
	void addSkyComponent(std::shared_ptr<SkyComponent> pComponent);
private:
	SkyComponentList _skyComponentList;

public:
	void addCollisionShapeComponent(std::shared_ptr<PrimitiveComponent> pComponent);
private:
	CollisionComponentList _collisionComponentList;

private:
	std::vector<std::shared_ptr<RenderTarget>> _renderTargetList;
	std::shared_ptr<RenderTarget> _lightDiffuseTarget;
	std::shared_ptr<RenderTarget> _lightSpecularTarget;

private:
	std::shared_ptr<MeshComponent> _pMeshComponent;
};

#define __SCENE_RENDERER_H__
#endif