#pragma once

#include "AssetEditor.h"

class MDynamicMeshPhysics;
class MSkeleton;

class MDynamicMeshPhysicsEditor : public MAssetEditor
{
public:
    MDynamicMeshPhysicsEditor(MObject* InObject);
    ~MDynamicMeshPhysicsEditor() = default;

public:
    virtual void Test() override;
    virtual void Render() override;

public:
    std::shared_ptr<MDynamicMeshPhysics> GetDynamicMeshPhysics() { return std::static_pointer_cast<MDynamicMeshPhysics>(Asset);  }
protected:
    DynamicMesh* dynamicMesh = nullptr;
};