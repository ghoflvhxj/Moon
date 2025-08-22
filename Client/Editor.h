#pragma once

#include "Core/Module/Module.h"

class Component;
class MMeshComponent;
class TerrainComponent;
class SphereComponent;
class MCamera;
class MActor;
class Player;
class StaticMeshComponent;
class MStaticMeshActor;

enum class EAxies
{
    X, Y, Z
};

enum class EGizmoMode
{
    Trans,
    Rot,
    Scale,
    Count
};

class MEditor : public MModule
{
public:
	explicit MEditor();
	virtual ~MEditor();

public:
	virtual bool Initialize() override;
	virtual void Update() override;
	virtual void Render() override;

public:
    bool IsPickable() const;
private:
    Vec3 GizmoOffset = VEC3ZERO;
    EAxies GizmoAxis;
    bool bSetGizmoOffset = false;
	bool bControlGizmo = false;
    EGizmoMode GizmoMode = EGizmoMode::Trans;
    Vec3 Prev = {};

    std::weak_ptr<Component> ClickedComp;

    float CameraSpeedScale = 1.f;

public:
    const FTypeDesc* EditAssetDesc = nullptr;
    class MAsset* EditAsset = nullptr;

    void DispatchContainer(const FTypeDesc* InElementTypeDesc, FVectorPropertyDesc* InContainerDesc, void* InObject);
    void DispatchArray(const FTypeDesc* InElementTypeDesc, FPropertyDesc* InPropertyDesc, void* InObject);
    void DispatchStruct(const FTypeDesc* InStructDesc, void* InObject);


    void HandleProperty(EType InType, const char* DisplayName, void* InData);
};