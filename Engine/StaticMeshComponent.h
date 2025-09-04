#pragma once
#include "MeshComponent.h"
#include "Core/Physics/PhysicsEnum.h"

class StaticMesh;

class ENGINE_DLL StaticMeshComponent : public MMeshComponent
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

    // 메시 관련
public:
    virtual void SetMesh(const std::wstring& InPath) override;
    virtual std::shared_ptr<StaticMesh> GetMesh() override { return Mesh; }
protected:
    std::shared_ptr<StaticMesh> Mesh;
    
public:
	void Temp(float y);
	void SetGravity(bool bGravity);

public:
	void SetMass(float NewMass);
    void SetAngularVelocity(float x, float y, float z);
    void SetVelocity(float x, float y, float z);

    REFLECT(StaticMeshComponent
        , PROPERTY_DELEGATE(Mesh, [&](StaticMeshComponent* InObject) {
            InObject->Reload();
        })
    )
};