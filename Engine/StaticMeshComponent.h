#pragma once
#include "PrimitiveComponent.h"
#include "Core/Physics/PhysicsEnum.h"

class StaticMesh;

class ENGINE_DLL StaticMeshComponent : public MPrimitiveComponent
{
public:
	using SceneComponent::setTranslation;
	using SceneComponent::setScale;

public:
	explicit StaticMeshComponent();
	explicit StaticMeshComponent(const std::wstring& FilePath);
	explicit StaticMeshComponent(const std::wstring& FilePath, bool bUsePhysX, bool bUseRigidStatic = true);
	virtual ~StaticMeshComponent();

public:
	virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;
	virtual const bool GetBoundingBox(std::shared_ptr<MBoundingBox> &boundingBox) override;
	virtual void setTranslation(const Vec3 &translation) override;
	virtual void setScale(const Vec3& InScale) override;
    virtual Mat4& getWorldMatrix() override;
protected:
    Mat4 DeformalMatrix = IDENTITYMATRIX;

public:
	virtual XMMATRIX GetRotationMatrix();

public:
    void SetMesh(const std::wstring& InPath);
	virtual std::shared_ptr<StaticMesh>& getStaticMesh();

private:
	std::shared_ptr<StaticMesh> _pStaticMesh = nullptr;

public:
	void Temp(float y);
	void SetGravity(bool bGravity);

    // 피직스 테스트용
public:
    void Clothing();

public:
	void SetMass(float NewMass);
    void SetAngularVelocity(float x, float y, float z);
    void SetVelocity(float x, float y, float z);

public:
    void SetPhysics(bool bInPhysics, bool bForce = false);
    void SetPhysicsSimulate(bool bInSimulate, bool bForce = false);
protected:
    bool bPhysics = true;
    bool bPhysicsSimulate = false;

public:
    void SetPhysicsType(EPhysicsType InPhysicsType) { PhysicsType = InPhysicsType; }
protected:
    EPhysicsType PhysicsType = EPhysicsType::Static;

	// 피직스
private:
    //physx::PxDeformableSurface* DeformableSurface = nullptr;
    uint32 VertexNum = 0;
    
    // 피직스 렌더링을 위한 임시 메터리얼.
    std::shared_ptr<MMaterial> MaterialForPhysX = nullptr;
    // 피직스 렌더링을 위한 임시 메시데이터
    std::shared_ptr<FMeshData> MeshDataForPhysX = nullptr;

protected:
    std::shared_ptr<class MPhysicsObject> PhysicsObject;
};