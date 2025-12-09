#pragma once

#include "Editor.h"
#include "Core/Delegate.h"

class MAsset;
class Renderer;
class MJoltPhysics;
class DynamicMesh;
class MSkeleton;
class MWindow;

class MAssetEditor : public MEditorBase
{
public:
    MAssetEditor(MObject* InObject);
	virtual ~MAssetEditor() = default;

public:
    virtual void HandleObject() override;
    virtual const std::wstring& GetPath() const override;
    virtual void Update() override;
    virtual void RenderUI() override;
   
protected:
	const FTypeDesc* AssetTypeDesc = nullptr;
	std::shared_ptr<MAsset> Asset = nullptr;
    std::shared_ptr<MAsset> WorkingAsset = nullptr;

public:
    std::shared_ptr< MJoltPhysics> GetJolt() { return WeakJolt.lock(); }
protected:
    std::weak_ptr<MJoltPhysics> WeakJolt;

protected:
    // 애셋을 들고있는 오브젝트
    MObject* AssetOwningObject = nullptr;

public:
	void HandleDynamicMesh(DynamicMesh* InDynamicMesh);
    void HandleSkeleton(MSkeleton* InSkeleton, std::function<void(uint32)> InContextMenu);
protected:
    int32 SKT_PID = -1; // 임시
    int32 SelectedJointIndex = 0;
    std::shared_ptr<MActor> DynamicMeshActor = nullptr;
    std::shared_ptr<MActor> Target = nullptr;

    // 카메라 컨트롤 용
    Vec2 PrevMousePos = {};

    bool bControl = false;
    Mat4 RotMat = {};
};