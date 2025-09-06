#pragma once
#include "MeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Mesh/DynamicMesh/DynamicMesh.h"

class ENGINE_DLL DynamicMeshComponent : public MMeshComponent
{
public:
	explicit DynamicMeshComponent();
	explicit DynamicMeshComponent(const std::wstring& FilePath);
	virtual ~DynamicMeshComponent();

public:
    virtual void BeginPlay() override;
    virtual void Update(const Time deltaTime);
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList) override;

public:
    virtual void SetMesh(const std::wstring& InPath) override;
    virtual std::shared_ptr<StaticMesh> GetMesh() override { return Mesh; }
protected:
    std::shared_ptr<DynamicMesh> Mesh;

public:
    virtual void Clothing() override;
    virtual void SetPhysics(bool bInPhysics, bool bForce = false);

public:
    // 조인트의 월드 위치를 반환함
    Vec3 GetJointPosition(uint32 JointIndex);
    Vec3 GetJointPosition(const std::string& InName);
    // 조인트의 컴포넌트 상대 위치를 반환함
    Vec3 GetRelativeJointPosition(const std::string& InName);
    Vec4 GetJointRotation(uint32 JointIndex);
    Vec4 GetJointRotation(const std::string& InName);
    Vec3 GetJointScale(uint32 InJointIndex);
public:
    void SetAnimClip(const uint32 Index);
    uint32 GetAnimClipNum();
	void playAnimation(const uint32 index, const Time deltaTime);
private:
	uint32 AinmClipIndex = 0;
	float AnimTime = 0.f;

public:
    Mat4* GetAnimMatrices() { return JointAnimMatrices; }
    Mat4 GetAnimMatrix(uint32 JointIndex);
    // 현재 프레임에서 조인트 행렬들
	Mat4 JointAnimMatrices[200];

public:
    void SetAnimPlaying(bool bPlaying) { bAnimPlaying = bPlaying; }
    bool IsAnimPlaying() const { return bAnimPlaying; }
protected:
    bool bAnimPlaying = false;
    bool bPlayAnimAtBegin = true;
    std::shared_ptr<MAnimation> Animation = nullptr;

public:
	std::shared_ptr<DynamicMesh> GetDynamicMesh();

    std::shared_ptr<MPhysicsObject> Kinematic;

    REFLECT(
        DynamicMeshComponent
        , PROPERTY(AnimTime)
        , PROPERTY(bAnimPlaying)
        , PROPERTY(bPlayAnimAtBegin)
        , PROPERTY(Animation)
        , PROPERTY_DELEGATE(Mesh, [&](DynamicMeshComponent* InObject) {
            InObject->Reload();
        })
    );
};