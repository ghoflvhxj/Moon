#pragma once

#include "AssetEditor.h"

class MDynamicMeshPhysics;
class MSkeleton;

class MDynamicMeshPhysicsEditor : public MAssetEditor
{
public:
    MDynamicMeshPhysicsEditor();
    virtual ~MDynamicMeshPhysicsEditor();

public:
    //virtual void Test() override;
    virtual void Update() override;

public:
    //virtual void SetAsset(std::shared_ptr<MAsset>& InAsset) override;

public:
    std::shared_ptr<MDynamicMeshPhysics> GetDynamicMeshPhysics() { return std::static_pointer_cast<MDynamicMeshPhysics>(Asset);  }
protected:
    DynamicMesh* dynamicMesh = nullptr;

    std::shared_ptr<class MWindow> T = nullptr;
};