#pragma once
#include "MeshComponent.h"
#include "DynamicMeshComponentUtility.h"

class DynamicMesh;

class ENGINE_DLL DynamicMeshComponent : public MMeshComponent
{
public:
	explicit DynamicMeshComponent();
	explicit DynamicMeshComponent(const std::wstring& FilePath);
	virtual ~DynamicMeshComponent();

public:
    virtual void Update(const Time deltaTime);
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList) override;

public:

public:
    void SetAnimClip(const uint32 Index) { _currentAinmClipIndex = Index, CurrentAnimTime = 0.f; }
    uint32 GetAnimClipNum();
	void playAnimation(const uint32 index, const Time deltaTime);
private:
	uint32 _currentAinmClipIndex = 0;
	float CurrentAnimTime = 0.f;
	Mat4 _matrices[200];

public:
    void SetAnimPlaying(bool bPlaying) { bAnimPlaying = bPlaying; }
    bool IsAnimPlaying() const { return bAnimPlaying; }
protected:
    bool bAnimPlaying = true;

public:
	std::shared_ptr<DynamicMesh> GetDynamicMesh();
};