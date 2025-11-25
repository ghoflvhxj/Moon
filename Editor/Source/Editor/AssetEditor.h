#pragma once

#include "Editor.h"
#include "Core/Delegate.h"

class MAsset;
class Renderer;
class MJoltPhysics;
class DynamicMesh;
class MSkeleton;
class MWindow;

class MAssetEditor
{
public:
    MAssetEditor(MObject* InObject);
	virtual ~MAssetEditor() = default;

public:
	virtual void Update();
    virtual void Render();
protected:
    virtual void Test();

public:
	virtual void SetAsset(std::shared_ptr<MAsset>& InAsset);
protected:
	const FTypeDesc* AssetTypeDesc = nullptr;
	std::shared_ptr<MAsset> Asset = nullptr;

public:
    std::shared_ptr<MRenderer> GetRenderer() { return WeakRenderer.lock(); }
    std::shared_ptr< MJoltPhysics> GetJolt() { return WeakJolt.lock(); }
protected:
    std::weak_ptr<MRenderer> WeakRenderer;
    std::weak_ptr<MJoltPhysics> WeakJolt;


public:
    const std::string& GetTitle() const { return Title; }
protected:
    // ImGui 타이틀에 출력될 문자열
    std::string Title;
    // ImGui 닫기 버튼 처리를 위한 변수
    bool bOpen = true;
    // 애셋을 들고있는 오브젝트
    MObject* AssetOwningObject = nullptr;

public:
	void HandleDynamicMesh(DynamicMesh* InDynamicMesh);
    void HandleSkeleton(MSkeleton* InSkeleton, std::function<void(uint32)> InContextMenu);
protected:
    int32 SKT_PID = -1; // 임시
    int32 SelectedJointIndex = 0;
    std::shared_ptr<MActor> DynamicMeshActor = nullptr;

public:
    FDelegate<void>& GetClosedDelegate() { return OnClosedDelegate; }
protected:
    FDelegate<void> OnClosedDelegate;

    std::shared_ptr<class MEditorBaseWindow> T = nullptr;
    std::shared_ptr<MWorld> W = nullptr;
    std::shared_ptr<MActor> Light = nullptr;
    std::shared_ptr<MActor> Target = nullptr;

    // 애셋 표시하는 액터 컨트롤
    Vec2 PrevMousePos = {};
    bool bControl = false;
    Mat4 RotMat = {};
};