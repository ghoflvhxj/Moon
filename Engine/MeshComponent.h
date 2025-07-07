#pragma once

#include "Include.h"
#include "PrimitiveComponent.h"
#include "Core/Physics/PhysicsEnum.h"

class StaticMesh;
class MPhysicsObject;

struct FTest
{
    uint32 MeshIndex;
    std::vector<float> InvMass;
};

class ENGINE_DLL MMeshComponent abstract : public MPrimitiveComponent
{
public:
	explicit MMeshComponent();
	virtual ~MMeshComponent();

    // 메시 관련
public:
    void SetMesh(const std::wstring& InPath);
    std::shared_ptr<StaticMesh> GetMesh();
protected:
    std::shared_ptr<StaticMesh> Mesh;

public:
    void AddForce(const Vec3& InForce);

/* 피직스 관련 */
public:
    virtual void Clothing();
public:
    void SetPhysics(bool bInPhysics, bool bForce = false);
    void SetPhysicsSimulate(bool bInSimulate, bool bForce = false);
    void SetPhysicsType(EPhysicsType InPhysicsType) { PhysicsType = InPhysicsType; }
protected:
    bool bPhysics = true;
    bool bPhysicsSimulate = false;
    EPhysicsType PhysicsType = EPhysicsType::Static;
    std::shared_ptr<MPhysicsObject> PhysicsObject;
    // 바디 테스트
    std::shared_ptr<MPhysicsObject> PhysicsObject2;

    std::vector<FTest> ClothData;
};
