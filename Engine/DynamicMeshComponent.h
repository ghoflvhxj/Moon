#pragma once
#include "MeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Mesh/DynamicMesh/DynamicMesh.h"

struct FFrameBlendData
{
    uint32 PrevFrame = 0;
    uint32 NextFrame = 1;
    float PrevFrameFactor = 0.f;
    float NextFrameFactor = 1.f;

    void Reset()
    {
        PrevFrame = 0;
        NextFrame = 1;
        PrevFrameFactor = 0.f;
        NextFrameFactor = 1.f;
    }
};

struct FAnimBlendData
{
    std::shared_ptr<MAnimation> PrevAnim = nullptr;
    float PrevFrame = 0;

    float BlendingTime = 0.f;
    float BlendTime = 0.1f;

    void Reset()
    {
        PrevAnim = nullptr;
        PrevFrame = 0.f;

        BlendingTime = 0.f;
        BlendTime = 0.3f;
    }
};

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
    virtual std::shared_ptr<MMesh> GetMesh() override;
    std::shared_ptr<DynamicMesh> GetDynamicMesh();
protected:
    std::shared_ptr<DynamicMesh> Mesh;

public:
    virtual void Clothing() override;
    void Clothing2();

public:
    bool SetAnim(std::shared_ptr<MAnimation> InAnim, float InBlendingTime = 0.2f);
    void SetAnimPlaying(bool bPlaying) { bAnimPlaying = bPlaying; }
    bool IsAnimPlaying() const { return IsAnimPlaying(Animation); }
    bool IsAnimPlaying(std::shared_ptr<MAnimation> InAnim) const { return Animation == InAnim && bAnimPlaying; }
    bool HasAnim() const { return Animation != nullptr; }
    bool IsLooped() const { return bLooped; }
protected:
    std::shared_ptr<MAnimation> Animation = nullptr;
    bool bAnimPlaying = false;
    bool bPlayAnimAtBegin = true;
    bool bLooped = false;

public:
    Mat4 GetJointMatrix(uint32 InJointIndex, bool bOption = false);
    Mat4 GetJointWorldMatrix(const std::string& InName);
    Mat4 GetJointWorldMatrix(uint32 InJointIndex);
    Mat4 GetBlendedJointMatrix(const FFrameBlendData& InBlendData, uint32 InJointIndex);
    Mat4 GetBlendedJointMatrix(const std::shared_ptr<MAnimation> InAnim, float InFrame, uint32 InJointIndex);
    const FJoint& GetJoint(const std::string& InName);
    const FJoint& GetJoint(uint32 InJointIndex);
    // 조인트의 로컬 축을 반환함
    Vec3 GetJointAxis(uint32 InJointIndex, uint32 InAxisIndex);
    // 조인트의 월드 위치를 반환함
    Vec3 GetJointPosition(uint32 InJointIndex, const Vec3& InOffset = VEC3ZERO);
    Vec3 GetJointPosition(const std::string& InName, const Vec3& InOffset = VEC3ZERO);
    // 조인트의 컴포넌트 상대 위치를 반환함
    Vec3 GetRelativeJointPosition(const std::string& InName);
    Vec4 GetJointQuaternion(uint32 InJointIndex);
    Vec4 GetJointQuaternion(const std::string& InName);
    Vec3 GetJointScale(uint32 InJointIndex);
private:
    FFrameBlendData FrameBlendData;
    FAnimBlendData AnimBlendData;
    std::vector<Mat4> PrevAnimMatrices;
private:
    float FloatFrame = 0.f;
    uint32 Frame = 0;
	float AnimTime = 0.f;
    float AnimSpeed = 1.f;

public:
    const std::vector<Mat4>& GetAnimMatrices() { return JointAnimMatrices; }
    Mat4 GetAnimMatrix(const std::string& InName);
    Mat4 GetAnimMatrix(uint32 InJointIndex);
protected:
    // 현재 프레임에서 조인트 행렬들
    std::vector<Mat4> JointAnimMatrices;


public:
    bool bBindPose = false;


    bool bRootMotion = false;

    REFLECT(
        DynamicMeshComponent
        , PROPERTY(AnimTime)
        , PROPERTY(AnimSpeed)
        , PROPERTY(bAnimPlaying)
        , PROPERTY(bPlayAnimAtBegin)
        , PROPERTY(Animation)
        , PROPERTY(bBindPose)
        , PROPERTY_DELEGATE(Mesh, [&](DynamicMeshComponent* InObject) {
            InObject->Reload();
        })
    );
};