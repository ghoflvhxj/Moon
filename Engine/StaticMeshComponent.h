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

public:
	virtual XMMATRIX GetRotationMatrix();

public:
	void Temp(float y);
	void SetGravity(bool bGravity);

public:
	void SetMass(float NewMass);
    void SetAngularVelocity(float x, float y, float z);
    void SetVelocity(float x, float y, float z);

	// 피직스
private:
    //physx::PxDeformableSurface* DeformableSurface = nullptr;
    uint32 VertexNum = 0;
    
    // 피직스 렌더링을 위한 임시 메터리얼.
    std::shared_ptr<MMaterial> MaterialForPhysX = nullptr;
    // 피직스 렌더링을 위한 임시 메시데이터
    std::shared_ptr<FMeshData> MeshDataForPhysX = nullptr;

    REFLECT(StaticMeshComponent)
};