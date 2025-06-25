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

public:
	// 렌더할 Primitive 추가
	void AddPrimitive(std::shared_ptr<MPrimitiveComponent> &pComponent);
protected:
    // 포워드 렌더링을 위한 PrimitiveID - FPrimitiveData 쌍을 저장함
	std::map<uint32, std::vector<FPrimitiveData>> ForwardPrimitiveDataMap;
    // 디퍼드 렌더링을 위한 PrimitiveID - FPrimitiveData 쌍을 저장함
	std::map<uint32, std::vector<FPrimitiveData>> DeferredPrimitiveDataMap;
    // PrimitiveID - 버텍스 버퍼 쌍을 저장함
	std::map<uint32, std::vector<std::shared_ptr<MVertexBuffer>>> VertexBufferMap;

    //임시
public:
    std::shared_ptr<MVertexBuffer> GetVertexBuffer(uint32 InId) {
        return VertexBufferMap[InId][0];
    }

public:
    const std::vector<FPrimitiveData>& GetPrimitives(EPrimitiveType InPrimitiveType) { return Primitives[InPrimitiveType]; }
    // PrimitiveType - PrimitiveData 쌍을 저장함
    std::map<EPrimitiveType, std::vector<FPrimitiveData>> Primitives;

private:
    void FrustumCulling();
public:
    const std::vector<FPrimitiveData>& GetRenderablePrimitiveData() const { return RenderablePrimitiveData; }
protected:
    // 컬링 후 남은 PrimitiveData
    std::vector<FPrimitiveData> RenderablePrimitiveData;

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
	std::vector<std::shared_ptr<MRenderPass>> RenderPasses;

public:
	void Render();
	void RenderScene();
	void RenderText();
private:
	void UpdateGlobalConstantBuffer(std::shared_ptr<MShader>& Shader);
    void UpdateTickConstantBuffer(std::shared_ptr<MShader>& Shader);

public:
	uint32 TotalPrimitiveNum = 0;
	uint32 ShownPrimitiveNum = 0;
	uint32 CulledPrimitiveNum = 0;

public:
	void toggleRenderTarget();
private:
	bool _drawRenderTarget;

public:
	const bool IsGlobalBufferDirty() const;
private:
	// 한 프레임 동안에 ConstantBuffer가 변경되었는지 여부를 판단하기 위한 변수
	bool _bDirtyConstant;

protected:
    std::vector<float> _cascadeDistance;
    std::vector<Vec3> LightPosition;
    std::vector<Mat4> LightViewProj;

public:
	RENDERER_OPTION(DrawCollision)
};

#define __RENDERER_H__
#endif