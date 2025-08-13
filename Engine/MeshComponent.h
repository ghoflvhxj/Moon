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
	explicit MMeshComponent();
	virtual ~MMeshComponent();

public:
    void Reload()
    {
        std::wstring temp = Mesh->GetAssetPath();
        SetMesh(temp);
    }

    // 메시 관련
public:
    void SetMesh(const std::wstring& InPath, bool bSetPhyiscs = true);
    std::shared_ptr<StaticMesh> GetMesh();
    FDelegate<void, std::shared_ptr<MPrimitiveComponent>>& GetMeshChangedDelegate() { return OnMeshChangedDelegate; }
protected:
    std::shared_ptr<StaticMesh> Mesh;
    FDelegate<void, std::shared_ptr<MPrimitiveComponent>> OnMeshChangedDelegate;
    //FObjectPath MeshPathTest;

public:
    void SetMaterial(uint32 InIndex, std::shared_ptr<MMaterial> InMaterial) { Materials[InIndex] = InMaterial; }
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
    bool IsPhysicsSimulating() const { return bPhysicsSimulate; }
    bool IsPhysicsEnable() const { return bPhysics; }
protected:
    bool bPhysics = true;
    bool bPhysicsSimulate = false;
    EPhysicsType PhysicsType = EPhysicsType::Static;
    //std::shared_ptr<MPhysicsObject> PhysicsObject;

public:
    std::shared_ptr<MPhysicsObject> PhysicsObject;
    // 바디 테스트
    std::shared_ptr<MPhysicsObject> BodyTestObject;

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
        //PROPERTY(Mesh)
        PROPERTY_DELEGATE(Mesh, [&](MMeshComponent* InObject) {
            InObject->Reload();
        }),
        PROPERTY(PhysicsType)
    );
};
