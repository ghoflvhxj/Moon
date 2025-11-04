#pragma once

#include "Include.h"
#include "PrimitiveComponent.h"
#include "Core/ObjectPath.h"
#include "Core/Physics/PhysicsEnum.h"
#include "Mesh/StaticMesh/StaticMesh.h"

class MPhysicsObject;

struct FClothUpdateData
{
    int32 ClothDataIndex = -1;
    Vec3 PreviousJointPos = VEC3ZERO;
};

class ENGINE_DLL MMeshComponent abstract : public MPrimitiveComponent
{
public:
	explicit MMeshComponent() = default;
	virtual ~MMeshComponent() = default;

public:
    void Reload()
    {
        std::wstring temp = GetMesh()->GetAssetPath();
        SetMesh(temp);
    }

public:
    virtual void SetMesh(const std::wstring& InPath) = 0;
    virtual std::shared_ptr<StaticMesh> GetMesh() = 0;
    FDelegate<void, std::shared_ptr<MPrimitiveComponent>>& GetMeshChangedDelegate() { return OnMeshChangedDelegate; }
protected:
    FDelegate<void, std::shared_ptr<MPrimitiveComponent>> OnMeshChangedDelegate;

public:
    void SetMaterial(uint32 InIndex, std::shared_ptr<MMaterial> InMaterial);
protected:
    std::vector<std::shared_ptr<MMaterial>> Materials;

public:
    void AddForce(const Vec3& InForce);

/* 피직스 관련 */
public:
    virtual void Clothing();
    void RemovePhysics();
public:
    virtual void SetPhysics(bool bInPhysics, bool bForce = false);
    void SetPhysicsSimulate(bool bInSimulate, bool bForce = false);
    void SetPhysicsType(EPhysicsType InPhysicsType) { PhysicsType = InPhysicsType; }
    EPhysicsType GetPhysicsType() const { return PhysicsType; }
    bool IsPhysicsEnable() const { return bPhysics; }
    bool IsPhysicsSimulating() const { return bPhysicsSimulate; }
protected:
    // 물리 효과 여부
    bool bPhysics = true;
    // 물리 효과의 시뮬레이션 실행 중인지 여부
    bool bPhysicsSimulate = false;
    EPhysicsType PhysicsType = EPhysicsType::Static;

public:
    std::shared_ptr<MPhysicsObject> PhysicsObject;

protected:
    // 옷감 피직스 오브젝트
    std::vector<std::shared_ptr<MPhysicsObject>> ClothPhysicsObjects;
    // 옷감 피직스 오브젝트 업데이트를 위한 데이터. 개수가 같아야 함.
    std::vector<FClothUpdateData> ClothUpdateDatas;

    REFLECT(
        MMeshComponent,
        PROPERTY_DELEGATE(bPhysicsSimulate, [&](MMeshComponent* InObject) {
            InObject->SetPhysicsSimulate(InObject->IsPhysicsSimulating());
        }),
        PROPERTY(PhysicsType),
        PROPERTY_DELEGATE(Materials, [&](MMeshComponent* InObject) {
            InObject->UpdatePrimitive();
        })
    );
};
