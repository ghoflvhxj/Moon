#pragma once

#include "Include.h"
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
	void AddPrimitive(std::shared_ptr<MPrimitiveComponent> pComponent);
    void MakeBuffer(FPrimitiveData& PrimitiveData);
    void MakeBuffer(std::shared_ptr<MPrimitiveComponent> InComponent);
protected:
    // 포워드 렌더링을 위한 PrimitiveID - FPrimitiveData 쌍을 저장함
	std::map<uint32, std::vector<FPrimitiveData>> ForwardPrimitiveDataMap;
    // 디퍼드 렌더링을 위한 PrimitiveID - FPrimitiveData 쌍을 저장함
	std::map<uint32, std::vector<FPrimitiveData>> DeferredPrimitiveDataMap;
    // PrimitiveID - 버텍스 버퍼 쌍을 저장함, ConstantBuffer는 쉐이더에서 하는데 모든 버퍼를 Renderer가 관리할지, Shader가 할지 
    std::map<uint32, std::vector<std::shared_ptr<MVertexBuffer>>> VertexBuffers;
    std::map<uint32, std::vector<std::shared_ptr<MIndexBuffer>>> IndexBuffers;

    //임시
public:
    std::shared_ptr<MVertexBuffer> GetVertexBuffer(uint32 InId, uint32 Offset = 0) {
        return VertexBuffers[InId][Offset];
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
	void addRenderTargetForDebug(ERenderTarget InRenderTarget);
	std::shared_ptr<RenderTarget>& GetRenderTarget(ERenderTarget RenderTarget) { return _renderTargets[static_cast<int32>(RenderTarget)]; }
private:
	RenderTargets _renderTargets;

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

private:
    std::unordered_map<ERenderTarget, std::shared_ptr<StaticMeshComponent>> DebugRenderTargetMehses;
	bool bDebugRenderTargets = true;

// Editor
public:
    bool bGizmo = false;
    Vec3 GizmoPos = VEC3ZERO;
protected:
    std::shared_ptr<StaticMeshComponent> GizmoMeshComp = nullptr;

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
